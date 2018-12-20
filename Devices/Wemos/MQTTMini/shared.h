#pragma once

// Pin mappings
// The I2C interface for the BME280 uses the standard Wemos pins which are 
// D1 (GPIO5) for clock and D2 (GPIO4) for data

// These pins are used by other modules
// Used in power.h
#define POWER_CONTROL_PIN 2

// Used in inputSwitch.h
#define CONFIG_INPUT_PIN 14

// Used in PixelControl.h
#define NEOPIXEL_CONTOL_PIN 15

// Used to define the pin to receive serial data from the Air Quality sensor
#define AIRQ_SERIAL_RX 12

// Used to define the pin to receive serial data from the GPS receiver
#define GPS_SERIAL_RX 13

// Used by ZPH01.h and SDS011.h

SoftwareSerial SensorSerial(AIRQ_SERIAL_RX, -1, false, 128);
SoftwareSerial GPS_Serial(GPS_SERIAL_RX, -1, false, 128);

enum DeviceStates { starting, wifiSetup, showStatus, active};

// Sensor settings
#define SDS011_SENSOR 1
#define ZPH01_SENSOR 2
#define MIN_AIRQ_SENSOR_NO 1
#define MAX_AIRQ_SENSOR_NO 2

// I2C address of the BME 280 sensor
#define BME280_I2C_ADDRESS 0x76

// Pixel display settings
#define MAX_NO_OF_PIXELS 64

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

	int airqSensorType;

	int airqLowLimit;
	int airqLowWarnLimit;
	int airqMidWarnLimit;
	int airqHighWarnLimit;
	int airQHighAlertLimit;

	int noOfPixels;
	float pixel_red;
	float pixel_green;
	float pixel_blue;

	byte checkByte2;
};

struct Device_Settings settings;

enum WiFiState { WiFiOff, WiFiStarting, WiFiScanning, WiFiWaitingForNextScan, WiFiConnecting, WiFiConnected, ShowingWifiConnected, WiFiConnectFailed, WiFiNotConnected,
	WiFiSetupStarting, WiFiSetupOff, WiFiSetupAwaitingClients, WiFiSetupServingPage, WiFiSetupProcessingResponse, WiFiSetupDone };

WiFiState wifiState;

enum ClockState { CLockAwaitingWiFi, ClockAwaitingSync, ClockRunning };

ClockState clockState;

enum MQTTState { AwaitingWiFi, ConnectingToMQTTServer, ShowingConnectedToMQTTServer, ShowingConnectToMQTTServerFailed, ConnectedToMQTTServer, ConnectToMQTTServerFailed };

MQTTState mqttState;

// From clock.h
bool pub_clock_valid;
uint8_t pub_hour, pub_minute, pub_second, pub_day_of_week;
uint8_t pub_day, pub_month;
uint16_t pub_year;


// Set by ZP01.h and SDS011
float pub_ppm_25;

// Set by SDS011
float pub_ppm_10;

// set to true when the next set of values are ready
boolean pub_air_values_ready;

// From bme280.h
float pub_temp;
float pub_pressure;
float pub_humidity;
// millis value at last update of temp, pressure and humidity values
boolean pub_bme_values_ready;

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

char * dayNames[] = { "", "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

char * monthNames[] = { "", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };


