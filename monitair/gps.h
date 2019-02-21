#pragma once

#include <Arduino.h>

#include "sensors.h"

#define GPS_ERROR_NO_DATA_RECEIVED -1
#define GPS_ERROR_NO_FIX -2

#define GPS_READING_LIFETIME_MSECS 5000
#define GPS_SERIAL_DATA_TIMEOUT_MILLIS 500

struct gpsReading {
	unsigned long lastGPSreadingMillis;
	double lattitude;
	double longitude;
};

int startGps(struct sensor * gpsSensor);
int updateGpsReading(struct sensor * gpsSensor);
int addGpsReading(struct sensor * gpsSensor, char * jsonBuffer, int jsonBufferSize);
void gpsStatusMessage(struct sensor * gpsSensor, char * buffer, int bufferLength);

