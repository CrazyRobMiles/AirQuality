#pragma once

#include "sensors.h"

#define AIRQ_READING_TIMEOUT_MSECS 5000
#define AIRQ_NO_DATA_RECEIVED -1
#define AIRQ_NO_READING_DECODED -2
#define AIRQ_NO_SENSOR_DETECTED_AT_START -3
#define AIRQ_SENSOR_NOT_RECOGNIZED_AT_START -4

#define AIRQ_READING_LIFETIME_MSECS 5000
#define AIRQ_SERIAL_DATA_TIMEOUT_MILLIS 500
#define AIRQ_AVERAGE_LIFETIME_MSECS 10000

struct airqualityReading {
	float pm25;
	float pm10;
	unsigned long lastAirqAverageMillis;
	float pm25Average;
	float pm10Average;
	// these are temporary values that are not for public use
	float pm25AvgTotal;
	float pm10AvgTotal;
	int averageCount;
	int readings;
	int errors;
};

int startAirq(struct sensor * airqSensor);
int updateAirqReading(struct sensor * airqSensor);
int addAirqReading(struct sensor * airqSensor, char * jsonBuffer, int jsonBufferSize);
void airqStatusMessage(struct sensor * airqSensor, char * buffer, int bufferLength);
