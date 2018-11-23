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

// Reading values shared by all the components
// The values are set by the reader components and 
// displayed by the lcd module
// As long as you call the sensor update methods
// regularly these will be kept up to date

// From SDS011.h
float pub_ppm_10;
float pub_ppm_25;
// set to true when the next set of values are ready
boolean pub_air_values_ready;

// From bme280.h
float pub_temp;
float pub_pressure;
float pub_humidity;
// millis value at last update of temp, pressure and humidity values
boolean pub_bme_values_ready;

// From rtc.h
uint8_t pub_hour, pub_minute, pub_second, pub_day_of_week;
uint8_t pub_day, pub_month;
uint16_t pub_year;

// From timing.h
// The millis value we need from the system timer when
// the next upate is required
// Using the internal timer for this at the moment
// May use this to sleep the processor and then re-awaken
// in future versions. Must use the compare method that 
// handles the wrap round of the millis value as the device 
// may be running for a long time.
// Any sensors with update times greater than this value
// have been updated.

uint32_t pub_millis_at_next_update;

#define TICKS_PER_LORA_UPDATE 200

#define TICKS_PER_MQTT_UPDATE 200

boolean pub_lora_force_send = false;
boolean pub_mqtt_force_send = false;

long pub_latitude_mdeg;
long pub_longitude_mdeg;
unsigned long pub_ticks_at_last_gps_update;
bool pub_got_gps_fix;
#define MILLIS_LIFETIME_OF_GPS_FIX 30000

byte safe_r = 0, safe_g = 0, safe_b = 0;

// The number of millis to the next update - if 0 or negative
// it means that a reading is due - updated by timing.h
int32_t pub_millis_to_next_update;

// Wire i2c interface shared by the clock and the bme280 
TwoWire heltecWire = TwoWire(1);

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
