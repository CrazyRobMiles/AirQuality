#include "sensors.h"
#include "airquality.h"
#include "bme280.h"
#include "clock.h"
#include "gps.h"
#include "pixels.h"
#include "settings.h"

struct sensor gpsSensor = { "GPS", 0,  startGps, updateGpsReading, addGpsReading, gpsStatusMessage, -1, false, NULL, 0 };
struct sensor bme280Sensor = { "BME280", 0, startBme280, updateBME280Reading, addBME280Reading, bme280StatusMessage, -1, false, NULL, 0 };
struct sensor airqSensor = { "Air quality", 0,  startAirq, updateAirqReading, addAirqReading, airqStatusMessage, -1, false, NULL, 0 };
struct sensor clockSensor = { "Clock", 0,  startClock, updateClockReading, addClockReading, clockStatusMessage, -1, false, NULL, 0 };

struct sensor * sensorList[] =
{
	&bme280Sensor,
	&airqSensor,
	&clockSensor,
	&gpsSensor
};

struct sensor * findSensorByName(char * name)
{
	for (int i = 0; i < sizeof(sensorList) / sizeof(struct sensor *); i++)
	{
		if (strcasecmp(sensorList[i]->sensorName, name) == 0)
		{
			return sensorList[i];
		}
	}
	return NULL;
}


#define SENSOR_STATUS_BUFFER_SIZE 300

char sensorStatusBuffer[SENSOR_STATUS_BUFFER_SIZE];

#define SENSOR_VALUE_BUFFER_SIZE 300

char sensorValueBuffer[SENSOR_VALUE_BUFFER_SIZE];

void startSensors()
{
	// start all the sensor managers
	for (int i = 0; i < sizeof(sensorList) / sizeof(struct sensor *); i++)
	{
		struct sensor * startSensor = sensorList[i];
		Serial.printf("Starting sensor %s: ", startSensor->sensorName);
		startSensor->startSensor(sensorList[i]);
		startSensor->getStatusMessage(startSensor, sensorStatusBuffer, SENSOR_STATUS_BUFFER_SIZE);
		Serial.printf("%s\n", sensorStatusBuffer);
		startSensor->beingUpdated = true;
		addStatusItem(startSensor->status == SENSOR_OK);
	}
}

void dumpSensorStatus()
{
	Serial.println("Sensors");
	unsigned long currentMillis = millis();
	for (int i = 0; i < sizeof(sensorList) / sizeof(struct sensor *); i++)
	{
		sensorList[i]->getStatusMessage(sensorList[i], sensorStatusBuffer, SENSOR_STATUS_BUFFER_SIZE);
		sensorValueBuffer[0] = 0; // empty the buffer string
		sensorList[i]->addReading(sensorList[i], sensorValueBuffer, SENSOR_VALUE_BUFFER_SIZE);
		Serial.printf("    %s  %s Active time(microsecs): ",
			sensorStatusBuffer, sensorValueBuffer);
		Serial.print(sensorList[i]->activeTime);
		Serial.print("  Millis since last reading: ");
		Serial.println(ulongDiff(currentMillis, sensorList[i]->millisAtLastReading));
	}
}

void updateSensors()
{
	for (int i = 0; i < sizeof(sensorList) / sizeof(struct sensor *); i++)
	{
		if (sensorList[i]->beingUpdated)
		{
			//Serial.print(sensorList[i]->sensorName);
			//Serial.print(' ');
			unsigned long startMicros = micros();
			sensorList[i]->updateSensor(sensorList[i]);
			sensorList[i]->activeTime = ulongDiff(micros(), startMicros);
		}
	}
}

void createSensorJson(char * buffer, int bufferLength)
{
	snprintf(buffer, bufferLength, "{ \"dev\":\"%s\"", settings.deviceName);

	for (int i = 0; i < sizeof(sensorList) / sizeof(struct sensor *); i++)
	{
		if (sensorList[i]->beingUpdated)
		{
			sensorList[i]->addReading(sensorList[i], buffer, bufferLength);
		}
	}
	snprintf(buffer, bufferLength, "%s}", buffer);
}

void displaySensorStatus()
{
	for (int i = 0; i < sizeof(sensorList) / sizeof(struct process *); i++)
	{
		sensor * displayProcess = sensorList[i];
		addStatusItem(displayProcess->status == SENSOR_OK);
	}
}
