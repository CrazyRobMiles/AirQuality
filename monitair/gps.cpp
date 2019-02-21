#include <Arduino.h>
#include "SoftwareSerial.h"

#include <MicroNMEA.h>

#include "utils.h"
#include "sensors.h"
#include "settings.h"
#include "processes.h"
#include "gps.h"

char nmeaBuffer[100];
MicroNMEA nmea(nmeaBuffer, sizeof(nmeaBuffer));

SoftwareSerial * gpsSerial = NULL;

unsigned long gpsReadingStartTime;

unsigned long lastGPSserialDataReceived;

void printUnknownSentence(const MicroNMEA& nmea)
{
	//	TRACELN();
	//	TRACE("Unknown sentence: ");
	//	TRACELN(nmea.getSentence());
}

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

	lastGPSserialDataReceived = millis();

	nmea.setUnknownSentenceHandler(printUnknownSentence);

	gpsSensor->status = GPS_ERROR_NO_DATA_RECEIVED;
	return gpsSensor->status;
}

int updateGpsReading(struct sensor * gpsSensor)
{
	struct gpsReading * activeGPSReading;
	activeGPSReading =
		(struct gpsReading *) gpsSensor->activeReading;

	unsigned long updateMillis = millis();

	if (gpsSerial->available() == 0)
	{
		if (gpsSensor->status != GPS_ERROR_NO_DATA_RECEIVED)
		{
			unsigned long timeSinceLastSerialData = ulongDiff(updateMillis, lastGPSserialDataReceived);

			// if the data timeout has expired, change the state
			if (timeSinceLastSerialData > GPS_SERIAL_DATA_TIMEOUT_MILLIS)
				gpsSensor->status = GPS_ERROR_NO_DATA_RECEIVED;
		}

		return gpsSensor->status;
	}

	// got some serial data - yay

	lastGPSserialDataReceived = updateMillis;

	if (gpsSensor->status == GPS_ERROR_NO_DATA_RECEIVED)
	{
		// Now have data - but no fix
		gpsSensor->status = GPS_ERROR_NO_FIX;
	}

	while (gpsSerial->available())
	{
		char ch = gpsSerial->read();
		
		nmea.process(ch);

		if (nmea.isValid())
		{
			activeGPSReading->lattitude = nmea.getLatitude() / 1000000.0;
			activeGPSReading->longitude = nmea.getLongitude() / 1000000.0;
			activeGPSReading->lastGPSreadingMillis = updateMillis;
			gpsSensor->status = SENSOR_OK;
		}
	}

	unsigned long millisSinceGPSupate = ulongDiff(updateMillis, activeGPSReading->lastGPSreadingMillis);
		
	if (millisSinceGPSupate > GPS_READING_LIFETIME_MSECS)
	{
		gpsSensor->status = GPS_ERROR_NO_FIX;
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
			jsonBuffer,
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
	case GPS_ERROR_NO_DATA_RECEIVED:
		snprintf(buffer, bufferLength, "GPS sensor no data received");
		break;
	case GPS_ERROR_NO_FIX:
		snprintf(buffer, bufferLength, "GPS sensor no fix");
		break;
	}
}
