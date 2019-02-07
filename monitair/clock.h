#pragma once

#include <Arduino.h>
#include <ezTime.h>
#include "sensors.h"

#define CLOCK_ERROR_NO_WIFI -1
#define CLOCK_ERROR_TIME_NOT_SET -2
#define CLOCK_ERROR_NEEDS_SYNC -3

#define CLOCK_SYNC_TIMEOUT 5

struct clockReading {
	int hour;
	int minute;
	int second;
	int day;
	int month;
	int year;
	int dayOfWeek;
};

extern char * dayNames[] ;

extern char * monthNames[] ;

int startClock(struct sensor * clockSensor);
int updateClockReading(struct sensor * clockSensor);
int addClockReading(struct sensor * clockSensor, char * jsonBuffer, int jsonBufferSize);
void clockStatusMessage(struct sensor * clockSensor, char * buffer, int bufferLength);
