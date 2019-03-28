#pragma once

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

#define WIFI_CONNECT_TIMEOUT_MILLIS 20000

#define WIFI_OK 0
#define WIFI_TURNED_OFF 1
#define WIFI_ERROR_NO_NETWORKS_FOUND -1
#define WIFI_ERROR_CONNECT_TIMEOUT -2
#define WIFI_ERROR_CONNECT_FAILED -3
#define WIFI_ERROR_NO_MATCHING_NETWORKS -4
#define WIFI_ERROR_DISCONNECTED -5

#define WIFI_CONNECT_RETRY_MILLS 5000

struct WiFiSetting
{
	char * wifiSsid;
	char * wifiPassword;
};

int startWifi(struct process * wifiProcess);
int stopWiFi(struct process * wifiProcess);
int updateWifi(struct process * wifiProcess);
void wifiStatusMessage(struct process * wifiProcess, char * buffer, int bufferLength);
