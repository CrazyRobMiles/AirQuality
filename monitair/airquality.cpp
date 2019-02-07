#include <Arduino.h>
#include <SoftwareSerial.h>

#include "utils.h"
#include "sensors.h"
#include "airquality.h"
#include "settings.h"
#include "processes.h"

SoftwareSerial * airqSensorSerial = NULL;
unsigned long airqReadingStartTime;

#define AIRQ_SENSOR_START_SEQUENCE_LENGTH 2

struct airqSensorStartSequence
{
	int sensorType;
	char sequence[AIRQ_SENSOR_START_SEQUENCE_LENGTH];
	int pos;
};

struct airqSensorStartSequence airqStartSequences[] =
{
	{ SDS011_SENSOR, {170,192}, 0 },
	{ ZPH01_SENSOR, {0xFF,0x18}, 0 }
};


int getSensorType()
{
	byte buffer;
	int value;

	airqReadingStartTime = millis();

	for (int i = 0; i < sizeof(airqStartSequences) / sizeof(struct airqSensorStartSequence); i++)
	{
		airqStartSequences[i].pos = 0;
	}

	while (true)
	{
		if (ulongDiff(millis(), airqReadingStartTime) > AIRQ_READING_TIMEOUT_MSECS)
		{
			return AIRQ_ERROR_TIMED_OUT;
		}

		if (airqSensorSerial->available() == 0)
		{
			delay(1);
			continue;
		}

		buffer = airqSensorSerial->read();

		value = int(buffer);

		for (int i = 0; i < sizeof(airqStartSequences) / sizeof(struct airqSensorStartSequence); i++)
		{
			if (value == airqStartSequences[i].sequence[airqStartSequences[i].pos])
			{
				airqStartSequences[i].pos++;
				if (airqStartSequences[i].pos == AIRQ_SENSOR_START_SEQUENCE_LENGTH)
				{
					// found a match to the sensor
					return airqStartSequences[i].sensorType;
				}
			}
			else
			{
				airqStartSequences[i].pos = 0;
			}
		}
	}
}

int startAirq(struct sensor * airqSensor)
{
	struct airqualityReading * airqualityActiveReading;

	if (airqSensor->activeReading == NULL)
	{
		airqualityActiveReading = new airqualityReading();
		airqSensor->activeReading = airqualityActiveReading;
	}
	else
	{
		airqualityActiveReading =
			(struct airqualityReading *) airqSensor->activeReading;
	}

	// Open the serial port
	if (airqSensorSerial == NULL)
	{
		airqSensorSerial = new  SoftwareSerial(settings.airqRXPinNo, -1, false, 128);
		airqSensorSerial->begin(9600);
	}

	int sensorType = getSensorType();

	// Turn off interrupts until we need a reading
	airqSensorSerial->enableRx(false);

	if (sensorType == AIRQ_ERROR_TIMED_OUT)
	{
		airqSensor->status = AIRQ_ERROR_TIMED_OUT;
		return AIRQ_ERROR_TIMED_OUT;
	}

	airqSensor->status = SENSOR_OK;
	return SENSOR_OK;
}

boolean getSDS011reading(airqualityReading * result)
{
	// turn on the serial port 
	airqSensorSerial->enableRx(true);

	airqReadingStartTime = millis();

	byte sds011Buffer;
	int sds011Value;
	int sds011Len = 0;
	int sds011Pm10Serial = 0;
	int sds011PM25Serial = 0;
	int sds011CalcChecksum;
	int sds011ChecksumOK = 0;
	boolean sds011GotValue = false;

	while (true)
	{
		if (ulongDiff(millis(), airqReadingStartTime) > AIRQ_READING_TIMEOUT_MSECS)
		{
			sds011GotValue = false;
			break;
		}

		if (airqSensorSerial->available() == 0)
		{
			activeSerialCompatibleDelay(1);
			continue;
		}

		sds011Buffer = airqSensorSerial->read();

		sds011Value = int(sds011Buffer);

		switch (sds011Len) {
		case (0): if (sds011Value != 170) { sds011Len = -1; }; break;
		case (1): if (sds011Value != 192) { sds011Len = -1; }; break;
		case (2): sds011PM25Serial = sds011Value; sds011CalcChecksum = sds011Value; break;
		case (3): sds011PM25Serial += (sds011Value << 8); sds011CalcChecksum += sds011Value; break;
		case (4): sds011Pm10Serial = sds011Value; sds011CalcChecksum += sds011Value; break;
		case (5): sds011Pm10Serial += (sds011Value << 8); sds011CalcChecksum += sds011Value; break;
		case (6): sds011CalcChecksum += sds011Value; break;
		case (7): sds011CalcChecksum += sds011Value; break;
		case (8): if (sds011Value == (sds011CalcChecksum % 256)) { sds011ChecksumOK = 1; }
				  else { sds011Len = -1; }; break;
		case (9): if (sds011Value != 171) { sds011Len = -1; }; break;
		}

		sds011Len++;

		if (sds011Len == 10 && sds011ChecksumOK == 1) {
			result->pm10 = (float)sds011Pm10Serial / 10.0;
			result->pm25 = (float)sds011PM25Serial / 10.0;
			sds011GotValue = true;
			break;
		}
	}

	// turn off the serial port 

	airqSensorSerial->enableRx(false);

	return sds011GotValue;
}

boolean getXPH01reading(airqualityReading * result)
{
	// turn on the serial port 
	airqSensorSerial->enableRx(true);

	airqReadingStartTime = millis();

	byte buffer;
	int value;
	int len = 0;
	float pm25_serial = 0;
	byte checksum_is;
	boolean gotValue = false;

	while (true)
	{
		if (ulongDiff(millis(), airqReadingStartTime) > AIRQ_READING_TIMEOUT_MSECS)
		{
			gotValue = false;
			break;
		}

		if (airqSensorSerial->available() == 0)
		{
			activeSerialCompatibleDelay(1);
			continue;
		}

		buffer = airqSensorSerial->read();

		value = int(buffer);

		switch (len) {
		case (0): if (value != 0xFF) { len = -1; }; break;
		case (1): if (value != 0x18) { len = -1; }
				  else { checksum_is = value; }; break;
		case (2): if (value != 0) { len = -1; }; break;
		case (3): pm25_serial = value; checksum_is += value; break;
		case (4): pm25_serial += (value / 100.0); checksum_is += value; break;
		case (5): checksum_is += value; break;
		case (6): if (value != 0x01) { len = -1; }
				  else { checksum_is += value; }; break;
		case (7): checksum_is += value; break;
		case (8):  break;
		}

		len++;

		if (len == 9)
		{
			checksum_is = (~checksum_is) + 1;
			if (value == checksum_is)
			{
				result->pm10 = -1;
				result->pm25 = pm25_serial;
				gotValue = true;
				break;
			}
		}
	}

	// turn off the serial port 

	airqSensorSerial->enableRx(false);

	return gotValue;
}

int updateAirqReading(struct sensor * airqSensor)
{
	boolean gotReading = false;

	// fake the reading

	struct airqualityReading * airqualityActiveReading =
		(airqualityReading *)airqSensor->activeReading;

	//airqualityActiveReading->pm10 = 10.0;
	//airqualityActiveReading->pm25 = 2.5;

	//activeSerialCompatibleDelay(1);

	//return SENSOR_OK;

	switch (settings.airqSensorType)
	{
	case 1: // sds011
		gotReading = getSDS011reading(airqualityActiveReading);
		break;

	case 2: // XPH01
		gotReading = getXPH01reading(airqualityActiveReading);
		break;
	}

	if (gotReading)
	{
		airqSensor->millisAtLastReading = millis();
		airqSensor->status = SENSOR_OK;
	}
	else
	{
		airqSensor->status = AIRQ_ERROR_TIMED_OUT;
	}
	return airqSensor->status;
}

int addAirqReading(struct sensor * airqSensor, char * jsonBuffer, int jsonBufferSize)
{
	if (airqSensor->status == SENSOR_OK)
	{
		struct airqualityReading * airqualityActiveReading =
			(airqualityReading *)airqSensor->activeReading;

		switch (settings.airqSensorType)
		{
		case 1: // sds011
			snprintf(jsonBuffer, jsonBufferSize, "%s,\"PM10\":%.2f,\"PM25\":%.2f",
				jsonBuffer,
				airqualityActiveReading->pm10, airqualityActiveReading->pm25);
			return SENSOR_OK;

		case 2: // XPH01
			snprintf(jsonBuffer, jsonBufferSize, "%s,\"PM25\":%.2f",
				jsonBuffer,
				airqualityActiveReading->pm25);
			return SENSOR_OK;
		}
	}

	return airqSensor->status;
}

void airqStatusMessage(struct sensor * airqSensor, char * buffer, int bufferLength)
{
	struct airqualityReading * airqualityActiveReading =
		(airqualityReading *)airqSensor->activeReading;

	switch (settings.airqSensorType)
	{
	case SDS011_SENSOR:
		snprintf(buffer, bufferLength, "SDS011 sensor:");
		break;
	case ZPH01_SENSOR:
		snprintf(buffer, bufferLength, "ZPH01 sensor:");
		break;
	default:
		snprintf(buffer, bufferLength, "Unknown sensor:");
		break;
	}

	switch (airqSensor->status)
	{
	case SENSOR_OK:
		snprintf(buffer, bufferLength, "%s OK", buffer);
		break;

	case SENSOR_OFF:
		snprintf(buffer, bufferLength, "%s OFF", buffer);
		break;

	case AIRQ_ERROR_TIMED_OUT:
		snprintf(buffer, bufferLength, "%s Timed Out", buffer);
		break;

	default:
		snprintf(buffer, bufferLength, "Airq status invalid");
		break;
	}
}
