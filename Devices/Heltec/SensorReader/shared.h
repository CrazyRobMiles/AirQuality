#pragma once

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

#define DEVICE_NAME_LENGTH 10

#define WIFI_SSID_LENGTH 30
#define WIFI_PASSWORD_LENGTH 30

#define MQTT_SERVER_NAME_LENGTH 100
#define MQTT_USER_NAME_LENGTH 100
#define MQTT_PASSWORD_LENGTH 200
#define MQTT_DEVICE_NAME_LENGTH 100
#define MQTT_PUBLISH_TOPIC_LENGTH 100
#define MQTT_SUBSCRIBE_TOPIC_LENGTH 100

#define SPLASH_LINE_LENGTH 15

#define LORA_KEY_LENGTH 16
#define LORA_EUI_LENGTH 8

struct WiFi_Setting
{
	char wifiSsid[WIFI_SSID_LENGTH];
	char wifiPassword[WIFI_PASSWORD_LENGTH];
};

#define NO_OF_WIFI_SETTINGS 5

struct Device_Settings
{
	int version;
	byte checkByte1;
	char deviceNane[DEVICE_NAME_LENGTH];

	WiFi_Setting wifiSettings[NO_OF_WIFI_SETTINGS];
	bool wiFiOn = true;
	char mqttServer[MQTT_SERVER_NAME_LENGTH];
	int mqttPort;
	char mqttUser[MQTT_USER_NAME_LENGTH];
	char mqttPassword[MQTT_PASSWORD_LENGTH];
	char mqttName[MQTT_DEVICE_NAME_LENGTH];
	char mqttPublishTopic[MQTT_PUBLISH_TOPIC_LENGTH];
	char mqttSubscribeTopic[MQTT_SUBSCRIBE_TOPIC_LENGTH];

	int seconds_per_mqtt_update;
	int seconds_per_mqtt_retry;

	int seconds_per_lora_update;
	bool lora_enabled = true;
	bool lora_abp_active = true;

	u4_t lora_abp_DEVADDR;
	u1_t lora_abp_NWKSKEY[LORA_KEY_LENGTH];
	u1_t lora_abp_APPSKEY[LORA_KEY_LENGTH];

	u1_t lora_otaa_APPKEY[LORA_KEY_LENGTH];
	u1_t lora_otaa_DEVEUI[LORA_EUI_LENGTH];
	u1_t lora_otaa_APPEUI[LORA_EUI_LENGTH];

	boolean mqtt_enabled = true;

	char splash_screen_top_line[SPLASH_LINE_LENGTH];
	char splash_screen_bottom_line[SPLASH_LINE_LENGTH];

	byte checkByte2;
};

struct Device_Settings settings;

