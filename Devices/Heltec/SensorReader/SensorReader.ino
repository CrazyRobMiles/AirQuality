#include <MicroNMEA.h>
#include <NeoPixelBus.h>
#include <NeoPixelBrightnessBus.h>
#include <NeoPixelAnimator.h>

#include <EEPROM.h>
#include <base64.h>

#include <PubSubClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <lmic.h>
#include "ArduinoJson-v5.13.2.h"

#include "HeltecMD_DS3231.h"
//#define DEBUG
//#define SECURE_SOCKETS

#ifdef DEBUG

#define TRACE(s) Serial.print(s)
#define TRACE_HEX(s) Serial.print(s, HEX)
#define TRACELN(s) Serial.println(s)
#define TRACE_HEXLN(s) Serial.println(s, HEX)

#else

#define TRACE(s) 
#define TRACE_HEX(s) 
#define TRACELN(s) 
#define TRACE_HEXLN(s) 

#endif

#include "utils.h"
#include "shared.h"


boolean send_to_lora();
boolean send_to_mqtt();

#include "commands.h"
#include "bme280.h"
#include "lcd.h"
#include "SDS011.h"
#include "rtc.h"
#include "menu.h"
#include "WiFiConnection.h"
#include "mqtt.h"
#include "lora.h"
#include "input.h"
#include "timing.h"
#include "GPS.h"

void setup() {
	// put your setup code here, to run once:
	Serial.begin(115200);
	heltecWire.begin(21, 22);
	setup_bme280();
	setup_lcd();
	setup_sensor();
	setup_input();
	setup_rtc();
	setup_menu();
	setup_commands();
	setup_timing();
	setup_lora();
	setup_wifi();
	setup_mqtt();
	setup_gps();
}

void loop()
{
	loop_sensor();
	loop_input();
	loop_bme280();
	loop_rtc();
	loop_menu();
	loop_timing();
	loop_wifi();
	loop_mqtt();
	loop_lora();
	loop_commands();
	loop_gps();
	// update the display last of all
	loop_lcd();
}
