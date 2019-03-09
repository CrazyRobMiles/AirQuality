#include <Arduino.h>
#include <DNSServer.h>
#include <WiFiUdp.h>Secure.h>
#include <WiFiServer.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include <ESP8266WiFiType.h>
#include <ESP8266WiFiSTA.h>
#include <ESP8266WiFiScan.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFiAP.h>
#include <ESP8266WiFi.h>

#include "configwifi.h"
#include "settings.h"

char configSSID[WIFI_SSID_LENGTH];

std::unique_ptr<DNSServer> dnsServer;
const byte DNS_PORT = 53;

int startWifiConfig(struct process * wifiConfigProcess)
{
	WiFi.mode(WIFI_AP);

	delay(100);

	WiFi.softAP(settings.deviceName);

	dnsServer.reset(new DNSServer());

	dnsServer->setErrorReplyCode(DNSReplyCode::NoError);

	delay(500);

	return PROCESS_OK;
}

int updateWifiConfig(struct process * wifiConfigProcess)
{
	return PROCESS_OK;
}

int stopWifiConfig(struct process * wifiConfigProcess)
{
	return PROCESS_OK;
}

void wifiConfigStatusMessage(struct process * wifiConfigProcess, char * buffer, int bufferLength)
{
	snprintf(buffer, bufferLength, "Soft AP running");
}
