#pragma once

#include "sensors.h"

#define AIRQ_READING_TIMEOUT_MSECS 5000
#define AIRQ_ERROR_TIMED_OUT -1
#define AIRQ_ERROR_NO_SENSOR -2

struct airqualityReading {
	float pm25;
	float pm10;
};

int startAirq(struct sensor * airqSensor);
int updateAirqReading(struct sensor * airqSensor);
int addAirqReading(struct sensor * airqSensor, char * jsonBuffer, int jsonBufferSize);
void airqStatusMessage(struct sensor * airqSensor, char * buffer, int bufferLength);
