#pragma once

#define GPS_ERROR_NOT_IMPLEMENTED -1

int startGps(struct sensor * gpsSensor)
{
	gpsSensor->status = GPS_ERROR_NOT_IMPLEMENTED;
	return gpsSensor->status;
}

int updateGpsReading(struct sensor * gpsSensor)
{
	return gpsSensor->status;
}

int addGpsReading(struct sensor * gpsSensor, char * jsonBuffer, int jsonBufferSize)
{
	return gpsSensor->status;
}

void gpsStatusMessage(struct sensor * gpsSensor, char * buffer, int bufferLength)
{
	snprintf(buffer, bufferLength, "GPS sensor not implemented");
}
