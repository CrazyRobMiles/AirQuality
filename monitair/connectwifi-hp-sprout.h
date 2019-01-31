#pragma once


#define WIFI_CONNECT_TIMEOUT_IN_SECONDS 20000

#define WIFI_OK 0
#define WIFI_ERROR_NO_NETWORKS 1
#define WIFI_ERROR_CONNECT_FAILED 2
#define WIFI_ERROR_CONNECT_TIMEOUT 3

struct WiFiSetting
{
	char * wifiSsid;
	char * wifiPassword;
};

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


boolean startWifi(struct process * wifiProcess)
{
	int setting_number;

	int noOfNetworks = WiFi.scanNetworks();

	if (noOfNetworks == 0)
	{
		wifiProcess->status = WIFI_ERROR_NO_NETWORKS;
		snprintf(wifiProcess->statusMessage, STATUS_DESCRIPTION_LENGTH, "No WiFi networks found");
		return false;
	}

	for (int i = 0; i < noOfNetworks; ++i) {
		setting_number = findWifiSetting(WiFi.SSID(i));
		if (setting_number != WIFI_SETTING_NOT_FOUND)
		{
			snprintf(wifiProcess->statusMessage, STATUS_DESCRIPTION_LENGTH, "Connect to %s ", wifiSettings[setting_number].wifiSsid);
			WiFi.begin(wifiSettings[setting_number].wifiSsid,
				wifiSettings[setting_number].wifiPassword);
			unsigned long connectStartTime = millis();

			while (WiFi.status() != WL_CONNECTED) 
			{
				activeDelay(500);
				if (ulongDiff(millis(), connectStartTime) > WIFI_CONNECT_TIMEOUT_IN_SECONDS)
				{
					WiFi.disconnect();
					snprintf(wifiProcess->statusMessage, STATUS_DESCRIPTION_LENGTH, "%s timeout", wifiProcess->statusMessage);
					wifiProcess->status = WIFI_ERROR_CONNECT_TIMEOUT;
					return false;
				}
			}

			if (WiFi.status() == WL_CONNECTED)
			{
				snprintf(wifiProcess->statusMessage, STATUS_DESCRIPTION_LENGTH, "%s connected ip address: %s", wifiProcess->statusMessage, WiFi.localIP().toString().c_str());
				return true;
			}

			snprintf(wifiProcess->statusMessage, STATUS_DESCRIPTION_LENGTH, "%s connect failed %d", wifiProcess->statusMessage, WiFi.status());

			wifiProcess->status = WIFI_ERROR_CONNECT_FAILED;
			return false;
		}
	}
	snprintf(wifiProcess->statusMessage, STATUS_DESCRIPTION_LENGTH, "No matching WiFi networks found");
	return false;
}

boolean stopWiFi(struct process * wifiProcess)
{
	wifiProcess->ActiveAtStart = false;

	return true;
}

boolean updateWifi(struct process * wifiProcess)
{
	if (!wifiProcess->ActiveAtStart)
		return true;

	// do the update behaviour here

	return true;
}

