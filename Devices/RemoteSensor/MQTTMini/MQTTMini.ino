#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ezTime.h>
#include <MicroNMEA.h>

// Uncomment to enable debug output
//#define DEBUG

// Uncomment this to enable secure sockets for use with Azure IOT Hub
// This will stop the device from aquiring the date and time from the ntp server
// as this seems to conflict with the use of the secure WiFi client

//#define SECURE_SOCKETS 

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
#include "gps.h"
#include "menu.h"
#include "commands.h"
#include "WiFiConnection.h"
#include "mqtt.h"
#include "AirQSensor.h"
#include "bme280.h"
#include "power.h"
#include "stateManager.h"


void setup() {
	// put your setup code here, to run once:
	Serial.begin(115200);
	delay(500);
	setup_commands(); // must call this first because it loads all the settings used by 
					  // all the elements
	setup_airq_sensor();
	setup_bme280();
	setup_power();
	setup_input();
	setup_wifi();
	setup_mqtt();
	setup_gps();
#ifndef SECURE_SOCKETS
	setup_clock();
#endif
	setup_timing();
	setup_StateManager();

	dump_settings();
}

void loop() {

	loop_input();
	loop_commands();

#ifndef SECURE_SOCKETS
	loop_clock();
#endif
	loop_mqtt();
	loop_wifi();
	loop_airq_sensor();
	loop_gps();
	loop_bme280();
	loop_power();
	loop_timing();
	loop_StateManager();
}
