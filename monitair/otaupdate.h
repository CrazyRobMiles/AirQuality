#pragma once

#define OTAUPDATE_OK 0
#define OTAUPDATE_OFF 1
#define OTAUPDATE_ERROR_NO_WIFI -1


int startOtaUpdate(struct process * otaUpdateProcess)
{
	if (WiFiProcessDescriptor.status != WIFI_OK)
	{
		otaUpdateProcess->status = OTAUPDATE_ERROR_NO_WIFI;
		return OTAUPDATE_ERROR_NO_WIFI;
	}
}
int updateOtaUpdate(struct process * otaUpdateProcess)
{
	return otaUpdateProcess->status;
}

int stopOtaUpdate(struct process * otaUpdateProcess)
{
	otaUpdateProcess->status = OTAUPDATE_OFF;
	return OTAUPDATE_OFF;
}

void otaUpdateStatusMessage(struct process * otaUpdateProcess, char * buffer, int bufferLength)
{
	switch (otaUpdateProcess->status)
	{
	case OTAUPDATE_OK:
		snprintf(buffer, bufferLength, "OTA update OK");
		break;
	case OTAUPDATE_OFF:
		snprintf(buffer, bufferLength, "OTA update OFF");
		break;
	case OTAUPDATE_ERROR_NO_WIFI:
		snprintf(buffer, bufferLength, "No Wifi for OTA update");
		break;
	default:
		snprintf(buffer, bufferLength, "OTA status invalid");
		break;
	}
}


