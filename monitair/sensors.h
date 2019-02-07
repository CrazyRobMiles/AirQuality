#pragma once

#include <Arduino.h>

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

struct sensor * findSensorByName(char * name);

void startSensors();

void dumpSensorStatus();

void updateSensors();

void createSensorJson(char * buffer, int bufferLength);

void displaySensorStatus();
