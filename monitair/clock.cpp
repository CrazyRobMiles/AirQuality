#include "clock.h"
#include "connectwifi.h"

struct process * clockWiFiProcess = NULL;

char * dayNames[] = { "", "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

char * monthNames[] = { "", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };


int startClock(struct sensor * clockSensor)
{
	if (clockWiFiProcess == NULL)
	{
		clockWiFiProcess = findProcessByName("WiFi");
	}

	struct clockReading * clockActiveReading;

	if (clockSensor->activeReading == NULL)
	{
		clockActiveReading = new clockReading();
		clockSensor->activeReading = clockActiveReading;
	}
	else
	{
		clockActiveReading =
			(struct clockReading *) clockSensor->activeReading;
	}

	if (clockWiFiProcess->status != WIFI_OK)
	{
		clockSensor->status = CLOCK_ERROR_NO_WIFI;
		return CLOCK_ERROR_NO_WIFI;
	}

	events();

	if (waitForSync(CLOCK_SYNC_TIMEOUT)) {
		clockSensor->status = SENSOR_OK;
		return SENSOR_OK;
	}

	clockSensor->status = CLOCK_ERROR_NEEDS_SYNC;
	return CLOCK_ERROR_NEEDS_SYNC;
}

void getClockReadings(struct sensor * clockSensor)
{
	struct clockReading * clockActiveReading;
	clockActiveReading =
		(struct clockReading *) clockSensor->activeReading;

	clockActiveReading->hour = hour();
	clockActiveReading->minute = minute();
	clockActiveReading->second = second();
	clockActiveReading->day = day();
	clockActiveReading->month = month();
	clockActiveReading->year = year();
	clockActiveReading->dayOfWeek = weekday();
	clockSensor->millisAtLastReading = millis();
}

int updateClockReading(struct sensor * clockSensor)
{
	events();

	switch (timeStatus())
	{
	case timeNotSet:
		clockSensor->status = CLOCK_ERROR_TIME_NOT_SET;
		break;
	case timeSet:
		getClockReadings(clockSensor);
		clockSensor->millisAtLastReading = millis();
		clockSensor->status = SENSOR_OK;
		break;
	case timeNeedsSync:
		if (waitForSync(CLOCK_SYNC_TIMEOUT)) {
			getClockReadings(clockSensor);
			clockSensor->status = SENSOR_OK;
		}
		else
		{
			clockSensor->status = CLOCK_ERROR_NEEDS_SYNC;
		}
		break;
	}

	return clockSensor->status;
}

int addClockReading(struct sensor * clockSensor, char * jsonBuffer, int jsonBufferSize)
{
	if (clockSensor->status == SENSOR_OK)
	{
		struct clockReading * clockActiveReading;
		clockActiveReading =
			(struct clockReading *) clockSensor->activeReading;

		snprintf(jsonBuffer, jsonBufferSize, "%s,\"timestamp\":\"%s %s %d %d %02d:%02d:%02d GMT+0000\"",
			jsonBuffer,
			dayNames[clockActiveReading->dayOfWeek],
			monthNames[clockActiveReading->month],
			clockActiveReading->day,
			clockActiveReading->year,
			clockActiveReading->hour,
			clockActiveReading->minute,
			clockActiveReading->second);
	}

	return clockSensor->status;
}

void clockStatusMessage(struct sensor * clockSensor, char * buffer, int bufferLength)
{
	switch (clockSensor->status)
	{
	case SENSOR_OK:
		snprintf(buffer, bufferLength, "Clock up to date");
		break;
	case CLOCK_ERROR_NO_WIFI:
		snprintf(buffer, bufferLength, "Clock error no wifi");
		break;
	case CLOCK_ERROR_TIME_NOT_SET:
		snprintf(buffer, bufferLength, "Clock error time not set");
		break;
	case CLOCK_ERROR_NEEDS_SYNC:
		snprintf(buffer, bufferLength, "Clock error time needs sync");
		break;
	default:
		break;
	}
}

