#include <Arduino.h>
#include <DNSServer.h>
#include <WiFiUdp.h>
#include <WiFiServerSecureBearSSL.h>
#include <WiFiServerSecureAxTLS.h>
#include <WiFiServerSecure.h>
#include <WiFiServer.h>
#include <WiFiClientSecureBearSSL.h>
#include <WiFiClientSecureAxTLS.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include <ESP8266WiFiType.h>
#include <ESP8266WiFiSTA.h>
#include <ESP8266WiFiScan.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFiAP.h>
#include <ESP8266WiFi.h>

#include "utils.h"
#include "processes.h"
#include "otaupdate.h"

struct process * otaWiFiProcess = NULL;


int startOtaUpdate(struct process * otaUpdateProcess)
{
	if (otaWiFiProcess == NULL)
	{
		otaWiFiProcess = findProcessByName("WiFi");
	}

	if (otaWiFiProcess->status != WIFI_OK)
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


