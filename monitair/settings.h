#pragma once

#include <Arduino.h>

#define MAJOR_VERSION 1
#define MINOR_VERSION 5

// Sensor settings
#define UNKNOWN_SENSOR 0
#define SDS011_SENSOR 1
#define ZPH01_SENSOR 2
#define PMS5003_SENSOR 3

#define DEVICE_NAME_LENGTH 20

#define WIFI_SSID_LENGTH 30
#define WIFI_PASSWORD_LENGTH 30

#define SERVER_NAME_LENGTH 200
#define MQTT_USER_NAME_LENGTH 100
#define MQTT_PASSWORD_LENGTH 200
#define MQTT_TOPIC_LENGTH 150
#define NUMBER_INPUT_LENGTH 20
#define YESNO_INPUT_LENGTH 0
#define ONOFF_INPUT_LENGTH 0
#define SETTING_ERROR_MESSAGE_LENGTH 120

#define MAX_SETTING_LENGTH 300

#define EEPROM_SIZE 5000
#define SETTINGS_EEPROM_OFFSET 0
#define CHECK_BYTE_O1 0xAA
#define CHECK_BYTE_O2 0x55

struct Device_Settings
{
	int majorVersion;
	int minorVersion;

	byte checkByte1;

	char deviceName[DEVICE_NAME_LENGTH];

	boolean indoorDevice;

	// WiFi settings

	char wifi1SSID[WIFI_SSID_LENGTH];
	char wifi1PWD[WIFI_PASSWORD_LENGTH];

	char wifi2SSID[WIFI_SSID_LENGTH];
	char wifi2PWD[WIFI_PASSWORD_LENGTH];

	char wifi3SSID[WIFI_SSID_LENGTH];
	char wifi3PWD[WIFI_PASSWORD_LENGTH];

	char wifi4SSID[WIFI_SSID_LENGTH];
	char wifi4PWD[WIFI_PASSWORD_LENGTH];

	char wifi5SSID[WIFI_SSID_LENGTH];
	char wifi5PWD[WIFI_PASSWORD_LENGTH];

	boolean wiFiOn;

	// Auto update settings

	char autoUpdateImageServer[SERVER_NAME_LENGTH];
	char autoUpdateStatusServer[SERVER_NAME_LENGTH];
	boolean autoUpdateEnabled;

	// MQTT settings

	char mqttServer[SERVER_NAME_LENGTH];
	boolean mqttSecureSockets;
	int mqttPort;
	char mqttUser[MQTT_USER_NAME_LENGTH];
	char mqttPassword[MQTT_PASSWORD_LENGTH];
	char mqttPublishTopic[MQTT_TOPIC_LENGTH];
	char mqttSubscribeTopic[MQTT_TOPIC_LENGTH];
	char mqttReportTopic[MQTT_TOPIC_LENGTH];

	int mqttSecsPerUpdate;
	int seconds_per_mqtt_retry;
	boolean mqtt_enabled;

	// Hardware settings

	int airqSensorType;
	int airqSecnondsSensorWarmupTime;
	int airqRXPinNo;
	boolean bme280Fitted;

	boolean powerControlFitted;
	int powerControlPin;
	int controlInputPin;
	boolean controlInputPinActiveLow;
	
	boolean gpsFitted;
	int gpsRXPinNo;

	boolean fixedLocation;
	double lattitude;
	double longitude;

	int pixelControlPinNo;
	int noOfPixels;

	int airqLowLimit;
	int airqLowWarnLimit;
	int airqMidWarnLimit;
	int airqHighWarnLimit;
	int airqHighAlertLimit;
	int airqNoOfAverages;


	int pixelRed;
	int pixelGreen;
	int pixelBlue;

	byte checkByte2;
};

extern struct Device_Settings settings;

enum Setting_Type { text, password, integerValue, doubleValue, onOff, yesNo };

struct SettingItem {
	char * prompt;
	char * formName;
	void * value;
	int maxLength;
	Setting_Type settingType;
	void (*setDefault)(void * destination);
	boolean (*validateValue)(void * dest, const char * newValueStr);
};

struct SettingItemCollection
{
	char * collectionName;
	char * collectionDescription;
	SettingItem * settings;
	int noOfSettings;
};

struct AllSystemSettings
{
	SettingItemCollection * collections;
	int noOfCollections;
};

enum processSettingCommandResult { displayedOK, setOK, settingNotFound, settingValueInvalid };

void saveSettings();
void loadSettings();
void resetSettings();
void PrintAllSettings();

SettingItemCollection * findSettingItemCollectionByName(const char * name);
AllSystemSettings * getAllSystemSettings();

processSettingCommandResult processSettingCommand(char * command);

void setupSettings();


