#include <DNSServer.h>
#include <ESP8266WebServerSecureBearSSL.h>
#include <ESP8266WebServerSecureAxTLS.h>
#include <ESP8266WebServerSecure.h>
#include <ESP8266WebServer.h>
#include <ezTime.h>

// Uncomment to enable debug output
//#define DEBUG

// Uncomment this to enable secure sockets for use with Azure IOT Hub
// This will stop the device from aquiring the date and time from the ntp server
// as this seems to conflict with the use of the secure WiFi client

//#define SECURE_SOCKETS 

#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>
#include <WiFiUdp.h>
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
#include <ESP8266WebServer.h>
#include <PubSubClient.h>
#include <SoftwareSerial.h>

#include "shared.h"
#include "utils.h"
#include "inputSwitch.h"
#include "ArduinoJson-v5.13.2.h"
#include "clock.h"
#include "timing.h"
#include "menu.h"
#include "commands.h"
#include "WiFiConnection.h"
#include "mqtt.h"
#include "ZPH01.h"
#include "PixelControl.h"
#include "WiFiConfig.h"
#include "stateManager.h"


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(100);
  Serial.println("Starting");
  WiFi.mode(WIFI_OFF);
  setup_zph01();
  setup_pixels();
  setup_input();
  setup_commands();
  setup_wifi();
  setup_mqtt();
#ifndef SECURE_SOCKETS
   setup_clock();
#endif
  setup_timing();
  setup_wifiConfig();
  setup_StateManager();
}

void loop() {
	loop_input();
	loop_commands();
#ifndef SECURE_SOCKETS
	loop_clock();
#endif
	loop_mqtt();
	loop_wifi();
	loop_zph01();
	loop_pixels();
	loop_timing();
	loop_wifiConfig();
	loop_StateManager();
}