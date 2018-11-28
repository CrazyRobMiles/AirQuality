#pragma once

enum DeviceStates { starting, wifiSetup, showStatus, active};

DeviceStates deviceState;

#define NO_OF_WIFI_SETTINGS 5
#define DEVICE_NAME_LENGTH 10

#define WIFI_SSID_LENGTH 30
#define WIFI_PASSWORD_LENGTH 30

#define MQTT_SERVER_NAME_LENGTH 100
#define MQTT_USER_NAME_LENGTH 100
#define MQTT_PASSWORD_LENGTH 200
#define MQTT_DEVICE_NAME_LENGTH 100
#define MQTT_PUBLISH_TOPIC_LENGTH 100
#define MQTT_SUBSCRIBE_TOPIC_LENGTH 100

struct WiFi_Setting
{
	char wifiSsid[WIFI_SSID_LENGTH];
	char wifiPassword[WIFI_PASSWORD_LENGTH];
};

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
	boolean mqtt_enabled = true;

	int airqLowLimit;
	int airqLowWarnLimit;
	int airqMidWarnLimit;
	int airqHighWarnLimit;
	int airQHighAlertLimit;

	byte checkByte2;
};

struct Device_Settings settings;

enum WiFiState { WiFiOff, WiFiStarting, WiFiScanning, WiFiConnecting, WiFiConnected, ShowingWifiConnected, WiFiConnectFailed, WiFiNotConnected };

WiFiState wifiState;

enum ClockState { CLockAwaitingWiFi, ClockAwaitingSync, ClockRunning };

ClockState clockState;

enum MQTTState { AwaitingWiFi, ConnectingToMQTTServer, ShowingConnectedToMQTTServer, ShowingConnectToMQTTServerFailed, ConnectedToMQTTServer, ConnectToMQTTServerFailed };

MQTTState mqttState;

enum WiFiSetupState { WiFiSetupOff, WiFiSetupAwaitingClients, WiFiSetupServingPage, WiFiSetupProcessingResponse};
WiFiSetupState wifiSetupState;

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

char * dayNames[] = { "", "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

char * monthNames[] = { "", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };


