#include <MicroNMEA.h>
#include <DNSServer.h>
#include <Adafruit_NeoPixel.h>
#include <ezTime.h>
#include <PubSubClient.h>
#include "SoftwareSerial.h"
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>
#include <ESP8266WebServerSecureBearSSL.h>
#include <ESP8266WebServerSecureAxTLS.h>
#include <ESP8266WebServerSecure.h>
#include <ESP8266WebServer.h>
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
#include <CertStoreBearSSL.h>
#include <BearSSLHelpers.h>
#include <EEPROM.h>
#include <Arduino.h>

#include "utils.h"
#include "settings.h"
#include "processes.h"
#include "sensors.h"
#include "bme280.h"
#include "airquality.h"
#include "connectwifi.h"
#include "configwifi.h"
#include "webserver.h"
#include "otaupdate.h"
#include "pixels.h"
#include "inputswitch.h"
#include "mqtt.h"
#include "clock.h"
#include "gps.h"
#include "console.h"
#include "control.h"

void setup() {
	Serial.begin(115200);
	delay(500);
	setupSettings();
	Serial.printf("\n\nMonitair node %s\nVersion %d.%d\n\n", settings.deviceName, 
		MAJOR_VERSION, MINOR_VERSION);

	startDevice();

	Serial.printf("\n\nType help and press enter for help\n\n");
}

void loop() 
{
	updateProcesses();
	updateSensors();
	sendSensorReadings();
}
