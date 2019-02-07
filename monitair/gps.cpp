#include <Arduino.h>
#include <SoftwareSerial.h>

#include <MicroNMEA.h>
#include <SoftwareSerial.h>

#include "utils.h"
#include "sensors.h"
#include "settings.h"
#include "processes.h"
#include "gps.h"

char nmeaBuffer[100];
MicroNMEA nmea(nmeaBuffer, sizeof(nmeaBuffer));

SoftwareSerial * gpsSerial;
unsigned long gpsReadingStartTime;

int startGps(struct sensor * gpsSensor)
{
	struct gpsReading * activeGPSReading ;

	if (gpsSensor->activeReading == NULL)
	{
		activeGPSReading = new gpsReading();
		gpsSensor->activeReading = activeGPSReading;
	}
	else
	{
		activeGPSReading =
			(struct gpsReading *) gpsSensor->activeReading;
	}

	// Open the serial port
	if (gpsSerial == NULL)
	{
		gpsSerial = new  SoftwareSerial(settings.gpsRXPinNo, -1, false, 128);
		gpsSerial->begin(9600);
	}

	gpsSensor->status = SENSOR_OK;
	return gpsSensor->status;
}

int updateGpsReading(struct sensor * gpsSensor)
{
	gpsReadingStartTime = millis();

	boolean noDataReceived = true;

	while (true)
	{
		if (ulongDiff(millis(), gpsReadingStartTime) > GPS_READING_TIMEOUT_MSECS)
		{
			if (noDataReceived)
			{
				gpsSensor->status = GPS_ERROR_NO_DATA_RECEIVED;
				break;
			}
			else
			{
				gpsSensor->status = GPS_ERROR_NO_FIX;
				break;
			}
		}

		if (gpsSerial->available() == 0)
		{
			activeSerialCompatibleDelay(1);
			continue;
		}

		while (gpsSerial->available())
		{
			noDataReceived = false;

			char ch = gpsSerial->read();
			nmea.process(ch);

			if (nmea.isValid())
			{
				struct gpsReading * activeGPSReading;
				activeGPSReading =
					(struct gpsReading *) gpsSensor->activeReading;
				activeGPSReading->lattitude = nmea.getLatitude() / 1000000.0;
				activeGPSReading->longitude = nmea.getLongitude() / 1000000.0;
				activeGPSReading->lastGPSreadingMillis = millis();
				gpsSensor->status = SENSOR_OK;
				return gpsSensor->status;
			}
		}
	}
	return gpsSensor->status;
}

int addGpsReading(struct sensor * gpsSensor, char * jsonBuffer, int jsonBufferSize)
{
	struct gpsReading * activeGPSReading;
	activeGPSReading =
		(struct gpsReading *) gpsSensor->activeReading;

	if (ulongDiff(millis(), activeGPSReading->lastGPSreadingMillis) < GPS_READING_LIFETIME_MSECS)
	{
		snprintf(jsonBuffer, jsonBufferSize, "%s,\"Lat\" : %.6f, \"Long\" : %.6f,",
			activeGPSReading->lattitude, activeGPSReading->longitude);
	}

	return gpsSensor->status;
}

void gpsStatusMessage(struct sensor * gpsSensor, char * buffer, int bufferLength)
{
	struct gpsReading * activeGPSReading;
	activeGPSReading =
		(struct gpsReading *) gpsSensor->activeReading;

	switch (gpsSensor->status)
	{
	case SENSOR_OK:
		if (ulongDiff(millis(), activeGPSReading->lastGPSreadingMillis) < GPS_READING_LIFETIME_MSECS)
			snprintf(buffer, bufferLength, "GPS sensor OK Lat: %.6f Long: %.6f Fix valid",
				activeGPSReading->lattitude, activeGPSReading->longitude);
		else
			snprintf(buffer, bufferLength, "GPS sensor OK Lat: %.6f Long: %.6f Fix timed out",
				activeGPSReading->lattitude, activeGPSReading->longitude);
		break;
	case GPS_ERROR_NOT_IMPLEMENTED:
		snprintf(buffer, bufferLength, "GPS sensor not implemented");
		break;
	case GPS_ERROR_NO_DATA_RECEIVED:
		snprintf(buffer, bufferLength, "GPS sensor no data received");
		break;
	case GPS_ERROR_NO_FIX:
		snprintf(buffer, bufferLength, "GPS sensor no fix");
		break;
	}
}
