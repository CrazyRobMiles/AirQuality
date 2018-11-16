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
#include <PubSubClient.h>
#include <SoftwareSerial.h>

// From clock.h
bool pub_clock_valid;
uint8_t pub_hour, pub_minute, pub_second, pub_day_of_week;
uint8_t pub_day, pub_month;
uint16_t pub_year;

// Set by ZP01.h
float pub_ppm_25;
// set to true when the next set of values are ready
boolean pub_air_values_ready;

// Used bymqtt.h and timing.h
bool pub_mqtt_force_send;
#define SECONDS_PER_MQTT_UPDATE 10
#define SECONDS_PER_MQTT_RETRY 1

// Set by WiFiGPS.h
long pub_latitude_mdeg;
long pub_longitude_mdeg;
unsigned long pub_ticks_at_last_gps_update;
bool pub_got_gps_fix;
#define MILLIS_LIFETIME_OF_GPS_FIX 30000

// Used by the pixel colours

byte safe_r = 0, safe_g = 0, safe_b = 0;

// Used by timing.h
uint32_t pub_millis_at_next_update;

// The number of millis to the next update - if 0 or negative
// it means that a reading is due - updated by timing.h
int32_t pub_millis_to_next_update;


#include "utils.h"
#include "ArduinoJson-v5.13.2.h"
#include "clock.h"
#include "timing.h"
#include "menu.h"
#include "commands.h"
#include "WiFiConnection.h"
#include "mqtt.h"
#include "ZPH01.h"
#include "PixelControl.h"

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Starting");
  setup_zph01();
  setup_pixels();
  setup_commands();
  setup_wifi();
  setup_mqtt();
#ifndef SECURE_SOCKETS
   setup_clock();
#endif
  setup_timing();
}

void loop() {
	loop_commands();
#ifndef SECURE_SOCKETS
	loop_clock();
#endif
	loop_mqtt();
	loop_wifi();
	loop_zph01();
	loop_pixels();
	loop_timing();
}
