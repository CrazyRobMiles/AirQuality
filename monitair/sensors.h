#pragma once

#define SENSOR_OK 0
#define SENSOR_OFF 1

struct sensor
{
	char * sensorName;
	unsigned long millisAtLastReading;
	int(*startSensor)(struct sensor *);
	int(*updateSensor)(struct sensor *);
	int(*addReading)(struct sensor *, char * jsonBuffer, int jsonBufferSize);
	void(*getStatusMessage)(struct sensor *, char * buffer, int bufferLength);
	int status;      // zero means OK - any other value is an error state
	boolean beingUpdated;  // active means that the sensor will be updated 
	void * activeReading;
	unsigned int activeTime;
};

int startBme280(struct sensor * bme280Sensor);
int updateBME280Reading(struct sensor * bme280Sensor);
int addBME280Reading(struct sensor * bme280Sensor, char * jsonBuffer, int jsonBufferSize);
void bme280StatusMessage(struct sensor * bme280sensor, char * buffer, int bufferLength);


int startAirq(struct sensor * airqSensor);
int updateAirqReading(struct sensor * airqSensor);
int addAirqReading(struct sensor * airqSensor, char * jsonBuffer, int jsonBufferSize);
void airqStatusMessage(struct sensor * airqSensor, char * buffer, int bufferLength);

int startClock(struct sensor * clockSensor);
int updateClockReading(struct sensor * clockSensor);
int addClockReading(struct sensor * clockSensor, char * jsonBuffer, int jsonBufferSize);
void clockStatusMessage(struct sensor * clockSensor, char * buffer, int bufferLength);

int startGps(struct sensor * gpsSensor);
int updateGpsReading(struct sensor * gpsSensor);
int addGpsReading(struct sensor * gpsSensor, char * jsonBuffer, int jsonBufferSize);
void gpsStatusMessage(struct sensor * gpsSensor, char * buffer, int bufferLength);

struct sensor bme280Sensor = { "BME280", 0, startBme280, updateBME280Reading, addBME280Reading, bme280StatusMessage, -1, false, NULL, 0 };
struct sensor airqSensor = { "Air quality", 0,  startAirq, updateAirqReading, addAirqReading, airqStatusMessage, -1, false, NULL, 0 };
struct sensor clockSensor = { "Clock", 0,  startClock, updateClockReading, addClockReading, clockStatusMessage, -1, false, NULL, 0 };
struct sensor gpsSensor = { "GPS", 0,  startGps, updateGpsReading, addGpsReading, gpsStatusMessage, -1, false, NULL, 0 };

struct sensor * sensorList[] =
{
	&bme280Sensor,
	&airqSensor,
	&clockSensor,
	&gpsSensor
};

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
			//Serial.printf("Updating %s\n", sensorList[i]->sensorName);
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
