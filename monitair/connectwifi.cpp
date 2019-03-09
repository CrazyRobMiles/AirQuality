#pragma once

#include "connectwifi.h"
#include "settings.h"

// Note that if we add new WiFi passwords into the settings
// we will have to update this table. 

struct WiFiSetting wifiSettings[] =
{
	{settings.wifi1SSID, settings.wifi1PWD},
	{settings.wifi2SSID, settings.wifi2PWD},
	{settings.wifi3SSID, settings.wifi3PWD},
	{settings.wifi4SSID, settings.wifi4PWD},
	{settings.wifi5SSID, settings.wifi5PWD}
};

#define NO_OF_WIFI_SETTINGS sizeof(wifiSettings) / sizeof(struct WiFiSetting)

#define WIFI_SETTING_NOT_FOUND -1

int findWifiSetting(String ssidName)
{
	char ssidBuffer[WIFI_SSID_LENGTH];

	ssidName.toCharArray(ssidBuffer, WIFI_SSID_LENGTH);

	for (int i = 0; i < NO_OF_WIFI_SETTINGS; i++)
	{
		if (strcasecmp(wifiSettings[i].wifiSsid, ssidBuffer) == 0)
		{
			return i;
		}
	}
	return WIFI_SETTING_NOT_FOUND;
}

char wifiActiveAPName[WIFI_SSID_LENGTH];
int wifiError;

boolean firstRun = true;
unsigned long lastWiFiConnectAtteptMillis;

int * wifiStatusAddress;

void onDisconnect(const WiFiEventStationModeDisconnected& event) {
	Serial.println("WiFi OnDisconnect.");
	*wifiStatusAddress = WIFI_ERROR_DISCONNECTED;
}

int startWifi(struct process * wifiProcess)
{
	wifiStatusAddress = &wifiProcess->status;

	lastWiFiConnectAtteptMillis = millis();
	int setting_number;
	Serial.println("Starting WiFi");

	// stop the device from being an access point when you don't want it 
	WiFi.onStationModeDisconnected(onDisconnect);
	if (firstRun)
	{
		Serial.println("First run");
		WiFi.mode(WIFI_OFF);
		delay(500);
		WiFi.mode(WIFI_STA);
		delay(500);
		firstRun = false;
	}

	int noOfNetworks = WiFi.scanNetworks();

	if (noOfNetworks == 0)
	{
		Serial.println("No networks");
		wifiProcess->status = WIFI_ERROR_NO_NETWORKS_FOUND;
		WiFi.scanDelete();
		return WIFI_ERROR_NO_NETWORKS_FOUND;
	}

	Serial.println("Networks found");

	for (int i = 0; i < noOfNetworks; ++i) {
		setting_number = findWifiSetting(WiFi.SSID(i));
		if (setting_number != WIFI_SETTING_NOT_FOUND)
		{
			snprintf(wifiActiveAPName, WIFI_SSID_LENGTH, "%s", wifiSettings[setting_number].wifiSsid);
			Serial.println(wifiActiveAPName);
			WiFi.begin(wifiSettings[setting_number].wifiSsid,
				wifiSettings[setting_number].wifiPassword);
			unsigned long connectStartTime = millis();

			while (WiFi.status() != WL_CONNECTED)
			{
				Serial.println("   waiting");
				delay(500);
				if (ulongDiff(millis(), connectStartTime) > WIFI_CONNECT_TIMEOUT_MILLIS)
				{
//					WiFi.disconnect();
					wifiProcess->status = WIFI_ERROR_CONNECT_TIMEOUT;
					Serial.println("Timeout");
					return WIFI_ERROR_CONNECT_TIMEOUT;
				}
			}

			Serial.println("Deleting scan");
			WiFi.scanDelete();

			wifiError = WiFi.status();

			if (wifiError == WL_CONNECTED)
			{
				Serial.println("Wifi OK");
				wifiProcess->status = WIFI_OK;
				return WIFI_OK;
			}

			Serial.print("Fail status:");
			Serial.println(wifiError, HEX);
			wifiProcess->status = WIFI_ERROR_CONNECT_FAILED;
			return WIFI_ERROR_CONNECT_FAILED;
		}
	}
	Serial.print("No matching networks");
	wifiProcess->status = WIFI_ERROR_NO_MATCHING_NETWORKS;
	return WIFI_ERROR_NO_MATCHING_NETWORKS;
}

int stopWiFi(struct process * wifiProcess)
{
	wifiProcess->status = WIFI_TURNED_OFF;
	return WIFI_TURNED_OFF;
}

int updateWifi(struct process * wifiProcess)
{
	if (wifiProcess->status == WIFI_OK)
	{
		int wifiStatusValue = WiFi.status();
		if (wifiStatusValue != WL_CONNECTED)
		{
			Serial.print("Disconnected code:");
			Serial.println(wifiStatusValue);
			wifiProcess->status = WIFI_ERROR_DISCONNECTED;
			lastWiFiConnectAtteptMillis = millis();
		}
	}
	else
	{
		unsigned long millisSinceWiFiConnectAttempt =
			ulongDiff(millis(), lastWiFiConnectAtteptMillis);

		if (millisSinceWiFiConnectAttempt > WIFI_CONNECT_RETRY_MILLS)
		{
			wifiProcess->status = startWifi(wifiProcess);
		}
	}

	return wifiProcess->status;
}

void wifiStatusMessage(struct process * wifiProcess, char * buffer, int bufferLength)
{
	switch (wifiProcess->status)
	{
	case WIFI_OK:
		snprintf(buffer, bufferLength, "%s connected ip address: %s", wifiActiveAPName, WiFi.localIP().toString().c_str());
		break;
	case WIFI_TURNED_OFF:
		snprintf(buffer, bufferLength, "Wifi OFF");
		break;
	case WIFI_ERROR_NO_NETWORKS_FOUND:
		snprintf(buffer, bufferLength, "No Wifi networks found");
		break;
	case WIFI_ERROR_CONNECT_TIMEOUT:
		snprintf(buffer, bufferLength, "%s connect timeout", wifiActiveAPName);
		break;
	case WIFI_ERROR_CONNECT_FAILED:
		snprintf(buffer, bufferLength, "%s connect failed with error %d", wifiActiveAPName, wifiError);
		break;
	case WIFI_ERROR_NO_MATCHING_NETWORKS:
		snprintf(buffer, bufferLength, "No stored Wifi networks found");
		break;
	case WIFI_ERROR_DISCONNECTED:
		snprintf(buffer, bufferLength, "WiFi disconnected");
		break;
	default:
		snprintf(buffer, bufferLength, "WiFi status invalid");
		break;
	}
}

