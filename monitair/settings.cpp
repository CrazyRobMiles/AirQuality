#include <EEPROM.h>
#include "settings.h"

struct Device_Settings settings;

void setEmptyString(void * dest)
{
	strcpy((char *)dest, "");
}

void setFalse(void * dest)
{
	boolean * destBool = (boolean *)dest;
	*destBool = false;
}

void setTrue(void * dest)
{
	boolean * destBool = (boolean *)dest;
	*destBool = true;
}

boolean validateOnOff(void * dest, const char * newValueStr)
{
	boolean * destBool = (boolean *)dest;

	if (strcasecmp(newValueStr, "on") == 0)
	{
		*destBool = true;
		return true;
	}

	if (strcasecmp(newValueStr, "off") == 0)
	{
		*destBool = false;
		return true;
	}

	return false;
}

boolean validateYesNo(void * dest, const char * newValueStr)
{
	boolean * destBool = (boolean *)dest;

	if (strcasecmp(newValueStr, "yes") == 0)
	{
		*destBool = true;
		return true;
	}

	if (strcasecmp(newValueStr, "no") == 0)
	{
		*destBool = false;
		return true;
	}

	return false;
}


boolean validateString(char * dest, const char * source, int maxLength)
{
	if (strlen(source) > (maxLength - 1))
		return false;

	strcpy(dest, source);

	return true;
}

boolean validateInt(void * dest, const char * newValueStr)
{
	int value;

	if (sscanf(newValueStr, "%d", &value) == 1)
	{
		*(int *)dest = value;
		return true;
	}

	return false;
}


boolean validateDouble(void * dest, const char * newValueStr)
{
	double value;

	if (sscanf(newValueStr, "%lf", &value) == 1)
	{
		*(double *)dest = value;
		return true;
	}

	return false;
}

void setDefaultDevname(void * dest)
{
	char * destStr = (char *)dest;
	snprintf(destStr, DEVICE_NAME_LENGTH, "CHASW-%06x-1", ESP.getChipId());
}


boolean validateDevName(void * dest, const char * newValueStr)
{
	return (validateString((char *)dest, newValueStr, DEVICE_NAME_LENGTH));
}

boolean validateWifiSSID(void * dest, const char * newValueStr)
{
	return (validateString((char *)dest, newValueStr, WIFI_SSID_LENGTH));
}

boolean validateWifiPWD(void * dest, const char * newValueStr)
{
	return (validateString((char *)dest, newValueStr, WIFI_PASSWORD_LENGTH));
}

struct SettingItem wifiSettingItems[] =
{
	"Device name", "devname", settings.deviceName, DEVICE_NAME_LENGTH, text, setDefaultDevname, validateDevName,
	"WiFiSSID1", "wifissid1", settings.wifi1SSID, WIFI_SSID_LENGTH, text, setEmptyString, validateWifiSSID,
	"WiFiPassword1", "wifipwd1", settings.wifi1PWD, WIFI_PASSWORD_LENGTH, password, setEmptyString, validateWifiPWD,
	"WiFiSSID2", "wifissid2", settings.wifi2SSID, WIFI_SSID_LENGTH, text, setEmptyString, validateWifiSSID,
	"WiFiPassword2", "wifipwd2", settings.wifi2PWD, WIFI_PASSWORD_LENGTH, password, setEmptyString, validateWifiPWD,
	"WiFiSSID3", "wifissid3", settings.wifi3SSID, WIFI_SSID_LENGTH, text, setEmptyString, validateWifiSSID,
	"WiFiPassword3", "wifipwd3", settings.wifi3PWD, WIFI_PASSWORD_LENGTH, password, setEmptyString, validateWifiPWD,
	"WiFiSSID4", "wifissid4", settings.wifi4SSID, WIFI_SSID_LENGTH, text, setEmptyString, validateWifiSSID,
	"WiFiPassword4", "wifipwd4", settings.wifi4PWD, WIFI_PASSWORD_LENGTH, password, setEmptyString, validateWifiPWD,
	"WiFiSSID5", "wifissid5", settings.wifi5SSID, WIFI_SSID_LENGTH, text, setEmptyString, validateWifiSSID,
	"WiFiPassword5", "wifipwd5", settings.wifi4PWD, WIFI_PASSWORD_LENGTH, password, setEmptyString, validateWifiPWD,
	"WiFi On", "wifion", &settings.wiFiOn, ONOFF_INPUT_LENGTH, onOff, setTrue, validateOnOff
};

boolean validateServerName(void * dest, const char * newValueStr)
{
	return (validateString((char *)dest, newValueStr, SERVER_NAME_LENGTH));
}

struct SettingItem autoUpdateSettingItems[] =
{
	"Auto update image server", "autoupdimage", settings.autoUpdateImageServer, SERVER_NAME_LENGTH, text, setEmptyString, validateServerName,
	"Auto update status server", "autoupdstatus", settings.autoUpdateStatusServer, SERVER_NAME_LENGTH, text, setEmptyString, validateServerName,
	"Auto update on", "autoupdon", &settings.autoUpdateEnabled, ONOFF_INPUT_LENGTH, onOff, setFalse, validateOnOff
};

char * defaultMQTTName = "NewMQTTDevice";
char * defaultMQTTHost = "mqtt.connectedhumber.org";

void setDefaultMQTTTname(void * dest)
{
	char * destStr = (char *)dest;
	snprintf(destStr, DEVICE_NAME_LENGTH, "Monitair-%06x", ESP.getChipId());
}

void setDefaultMQTThost(void * dest)
{
	strcpy((char *)dest, "mqtt.connectedhumber.org");
}

boolean validateMQTThost(void * dest, const char * newValueStr)
{
	return (validateString((char *)dest, newValueStr, SERVER_NAME_LENGTH));
}

void setDefaultMQTTport(void * dest)
{
	int * destInt = (int *)dest;
	*destInt = 1883; // use 8883 for secure connection to Azure IoT hub
}

boolean validateMQTTtopic(void * dest, const char * newValueStr)
{
	return (validateString((char *)dest, newValueStr, MQTT_TOPIC_LENGTH));
}

void setDefaultMQTTusername(void * dest)
{
	strcpy((char *)dest, "connectedhumber");
}

boolean validateMQTTusername(void * dest, const char * newValueStr)
{
	return (validateString((char *)dest, newValueStr, MQTT_USER_NAME_LENGTH));
}

boolean validateMQTTPWD(void * dest, const char * newValueStr)
{
	return (validateString((char *)dest, newValueStr, MQTT_PASSWORD_LENGTH));
}

void setDefaultMQTTpublishTopic(void * dest)
{
	snprintf((char *)dest, MQTT_TOPIC_LENGTH, "airquality/data/Monitair-%06x", ESP.getChipId());
}

void setDefaultMQTTsubscribeTopic(void * dest)
{
	snprintf((char *)dest, MQTT_TOPIC_LENGTH, "airquality/command/Monitair-%06x", ESP.getChipId());
}

void setDefaultMQTTreportTopic(void * dest)
{
	snprintf((char *)dest, MQTT_TOPIC_LENGTH, "airquality/report/Monitair-%06x", ESP.getChipId());
}

void setDefaultMQTTsecsPerUpdate(void * dest)
{
	int * destInt = (int *)dest;
	*destInt = 360;
}

void setDefaultMQTTsecsPerRetry(void * dest)
{
	int * destInt = (int *)dest;
	*destInt = 10;
}

struct SettingItem mqtttSettingItems[] =
{
	"MQTT Host", "mqtthost", settings.mqttServer, SERVER_NAME_LENGTH, text, setDefaultMQTThost, validateServerName,
	"MQTT port number", "mqttport", &settings.mqttPort, NUMBER_INPUT_LENGTH, integerValue,setDefaultMQTTport,validateInt,
	"MQTT secure sockets (on or off)", "mqttsecure", &settings.mqttSecureSockets, ONOFF_INPUT_LENGTH, onOff,setFalse, validateOnOff,
	"MQTT Active", "mqttactive", &settings.mqtt_enabled, ONOFF_INPUT_LENGTH, onOff, setTrue,validateOnOff,
	"MQTT UserName", "mqttuser", settings.mqttUser, MQTT_USER_NAME_LENGTH, text, setDefaultMQTTusername,validateMQTTusername,
	"MQTT Password", "mqttpwd", settings.mqttPassword, MQTT_PASSWORD_LENGTH, password, setEmptyString,validateMQTTPWD,
	"MQTT Publish topic", "mqttpub", settings.mqttPublishTopic, MQTT_TOPIC_LENGTH, text, setDefaultMQTTpublishTopic,validateMQTTtopic,
	"MQTT Subscribe topic", "mqttsub", settings.mqttSubscribeTopic, MQTT_TOPIC_LENGTH, text, setDefaultMQTTsubscribeTopic,validateMQTTtopic,
	"MQTT Reporting topic", "mqttreport", settings.mqttReportTopic, MQTT_TOPIC_LENGTH, text, setDefaultMQTTreportTopic,validateMQTTtopic,
	"MQTT Seconds per update", "mqttsecsperupdate", &settings.mqttSecsPerUpdate, NUMBER_INPUT_LENGTH, integerValue, setDefaultMQTTsecsPerUpdate, validateInt,
	"MQTT Seconds per retry", "mqttsecsperretry", &settings.seconds_per_mqtt_retry, NUMBER_INPUT_LENGTH, integerValue, setDefaultMQTTsecsPerRetry, validateInt
};

void setDefaultPixelRed(void * dest)
{
	int * destInt = (int *)dest;
	*destInt = 0;
}

void setDefaultPixelGreen(void * dest)
{
	int * destInt = (int *)dest;
	*destInt = 255;
}

void setDefaultPixelBlue(void * dest)
{
	int * destInt = (int *)dest;
	*destInt = 0;
}

boolean validateColour(void * dest, const char * newValueStr)
{
	int value;

	if (sscanf(newValueStr, "%d", &value) == 1)
	{
		*(int *)dest = value;
		return true;
	}

	if (value < 0)
		return false;

	if (value > 255)
		return false;

	return true;
}


void setDefaultAirqLowLimit(void * dest)
{
	int * destInt = (int *)dest;
	*destInt = 15;
}

void setDefaultAirqLowWarnLimit(void * dest)
{
	int * destInt = (int *)dest;
	*destInt = 40;
}


void setDefaultAirqMidWarnLimit(void * dest)
{
	int * destInt = (int *)dest;
	*destInt = 65;
}

void setDefaultAirqHighWarnLimit(void * dest)
{
	int * destInt = (int *)dest;
	*destInt = 150;
}

void setDefaultAirqHighAlertLimit(void * dest)
{
	int * destInt = (int *)dest;
	*destInt = 250;
}

void setDefaultAirqnoOfAverages(void * dest)
{
	int * destInt = (int *)dest;
	*destInt = 5;
}

void setDefault(void * dest)
{
	int * destInt = (int *)dest;
	*destInt = 0;
}

struct SettingItem pixelSettingItems[] =
{
	"Pixel red (0-255)", "pixelred", &settings.pixelRed, NUMBER_INPUT_LENGTH, integerValue, setDefaultPixelRed, validateColour,
	"Pixel green (0-255)", "pixelgreen", &settings.pixelGreen, NUMBER_INPUT_LENGTH, integerValue, setDefaultPixelGreen, validateColour,
	"Pixel blue (0-255)", "pixelblue", &settings.pixelBlue, NUMBER_INPUT_LENGTH, integerValue, setDefaultPixelBlue, validateColour,
	"AirQ Low Limit", "airqlowlimit", &settings.airqLowLimit, NUMBER_INPUT_LENGTH, integerValue, setDefaultAirqLowLimit, validateInt,
	"AirQ Low Warning Limit", "airqlowwarnlimit", &settings.airqLowWarnLimit, NUMBER_INPUT_LENGTH, integerValue, setDefaultAirqLowWarnLimit, validateInt,
	"AirQ Mid Warning Limit", "airqmidwarnlimit", &settings.airqMidWarnLimit, NUMBER_INPUT_LENGTH, integerValue, setDefaultAirqMidWarnLimit, validateInt,
	"AirQ High Warning Limit", "airqhighwarnlimit", &settings.airqHighWarnLimit, NUMBER_INPUT_LENGTH, integerValue, setDefaultAirqHighWarnLimit, validateInt,
	"AirQ High Alert Limit", "airqhighalertlimit", &settings.airqHighAlertLimit, NUMBER_INPUT_LENGTH, integerValue, setDefaultAirqHighAlertLimit, validateInt,
	"AirQ Number of averages", "airqnoOfAverages", &settings.airqNoOfAverages, NUMBER_INPUT_LENGTH, integerValue, setDefaultAirqnoOfAverages, validateInt,
};

void setDefaultAirQSensorType(void * dest)
{
	int * destInt = (int *)dest;
	*destInt = 0;
}

void setDefaultAirQWarmupTime(void * dest)
{
	int * destInt = (int *)dest;
	*destInt = 30;
}

void setDefaultAirqRXpin(void * dest)
{
	int * destInt = (int *)dest;
	*destInt = 12;
}

void setDefaultGpsPinNo(void * dest)
{
	int * destInt = (int *)dest;
	*destInt = 13;
}

void setDefaultPowerControlPinNo(void * dest)
{
	int * destInt = (int *)dest;
	*destInt = 2;
}

void setDefaultControlInputPin(void * dest)
{
	int * destInt = (int *)dest;
	*destInt = 14;
}


void setDefaultPixelControlPinNo(void * dest)
{
	int * destInt = (int *)dest;
	*destInt = 15;
}

void setDefaultNoOfPixels(void * dest)
{
	int * destInt = (int *)dest;
	*destInt = 12;
}

struct SettingItem hardwareSettingItems[] =
{
	"AirQ Sensor type (0 = not fitted 1=SDS011, 2=ZPH01)", "airqsensortype", &settings.airqSensorType, NUMBER_INPUT_LENGTH, integerValue, setDefaultAirQSensorType, validateInt,
	"AirQ Seconds for sensor warmup", "airqsensorwarmup", &settings.airqSecnondsSensorWarmupTime, NUMBER_INPUT_LENGTH, integerValue, setDefaultAirQWarmupTime, validateInt,
	"AirQ RX Pin", "airqrxpinno", &settings.airqRXPinNo, NUMBER_INPUT_LENGTH, integerValue, setDefaultAirqRXpin, validateInt,
	"BME 280 fitted", "bme280fitted", &settings.bme280Fitted, ONOFF_INPUT_LENGTH, yesNo, setTrue,validateYesNo,
	"Power Control fitted", "powercontrolfitted", &settings.powerControlFitted, ONOFF_INPUT_LENGTH, yesNo, setTrue,validateYesNo,
	"Power Control Pin", "powercontrolpin", &settings.powerControlPin, NUMBER_INPUT_LENGTH, integerValue, setDefaultPowerControlPinNo, validateInt,
	"Control Input Pin", "controlinputpin", &settings.controlInputPin, NUMBER_INPUT_LENGTH, integerValue, setDefaultControlInputPin, validateInt,
	"Control Input Active Low", "controlinputlow", &settings.controlInputPinActiveLow, ONOFF_INPUT_LENGTH, yesNo, setTrue, validateYesNo,
	"GPS fitted", "gpsfitted", &settings.gpsFitted, ONOFF_INPUT_LENGTH, yesNo, setFalse,validateYesNo,
	"GPS RX Pin", "gpsrxpin", &settings.gpsRXPinNo, NUMBER_INPUT_LENGTH, integerValue, setDefaultGpsPinNo, validateInt,
	"Number of pixels (0 for pixels not fitted)", "noofpixels", &settings.noOfPixels, NUMBER_INPUT_LENGTH, integerValue, setDefaultNoOfPixels, validateInt,
	"Pixel Control Pin", "pixelcontrolpin", &settings.pixelControlPinNo, NUMBER_INPUT_LENGTH, integerValue, setDefaultPixelControlPinNo, validateInt,
};

void setDefaultPositionValue(void * dest)
{
	double * destDouble = (double *)dest;
	*destDouble = -1000;
}

struct SettingItem locationSettingItems[] =
{
	"Fixed location", "fixedlocation", &settings.fixedLocation, ONOFF_INPUT_LENGTH, yesNo, setTrue,validateYesNo,
	"Device lattitude", "lattitude", &settings.lattitude, NUMBER_INPUT_LENGTH, doubleValue, setDefaultPositionValue, validateDouble,
	"Device longitude", "longitude", &settings.longitude, NUMBER_INPUT_LENGTH, doubleValue, setDefaultPositionValue, validateDouble,
	"Device indoors", "indoorDevice", &settings.indoorDevice, ONOFF_INPUT_LENGTH, yesNo, setFalse,validateYesNo
};


struct SettingItem quickSettingItems[] =
{
	"WiFi access point name", "wifissid1", settings.wifi1SSID, WIFI_SSID_LENGTH, text, setEmptyString, validateWifiSSID,
	"WiFi password", "wifipwd1", settings.wifi1PWD, WIFI_PASSWORD_LENGTH, password, setEmptyString, validateWifiPWD,
	"MQTT Password", "mqttpwd", settings.mqttPassword, MQTT_PASSWORD_LENGTH, password, setEmptyString,validateMQTTPWD,
	"Device lattitude", "lattitude", &settings.lattitude, NUMBER_INPUT_LENGTH, doubleValue, setDefaultPositionValue, validateDouble,
	"Device longitude", "longitude", &settings.longitude, NUMBER_INPUT_LENGTH, doubleValue, setDefaultPositionValue, validateDouble,
	"Device indoors", "indoorDevice", &settings.indoorDevice, ONOFF_INPUT_LENGTH, yesNo, setFalse,validateYesNo
};

SettingItemCollection allSettings[] = {
	{"Quick", "Just the settings to get you started", quickSettingItems, sizeof(quickSettingItems) / sizeof(SettingItem) },
	{"Wifi", "Set the SSID and password for wifi connections", wifiSettingItems, sizeof(wifiSettingItems) / sizeof(SettingItem) },
	{"MQTT", "Set the device, user, site, password and topic for MQTT", mqtttSettingItems, sizeof(mqtttSettingItems) / sizeof(SettingItem) },
	{"Pixel", "Set the pixel colours and display levels", pixelSettingItems, sizeof(pixelSettingItems) / sizeof(SettingItem)},
	{"Hardware", "Set the hardware pins and configuration", hardwareSettingItems, sizeof(hardwareSettingItems) / sizeof(SettingItem)},
	{"Location", "Set the fixed location of the device", locationSettingItems, sizeof(locationSettingItems) / sizeof(SettingItem)}
};

SettingItem * FindSetting(char * settingName)
{
	// Start the search at setting collection 1 so that the quick settings are not used in the search
	for (int collectionNo = 1; collectionNo < sizeof(allSettings) / sizeof(SettingItemCollection); collectionNo++)
	{
		for (int settingNo = 0; settingNo < allSettings[collectionNo].noOfSettings; settingNo++)
		{
			if (strcasecmp(settingName, allSettings[collectionNo].settings[settingNo].formName))
			{
				return &allSettings[collectionNo].settings[settingNo];
			}
		}
	}
	return NULL;
}

void printSetting(SettingItem * item)
{
	int * intValuePointer;
	boolean * boolValuePointer;
	double * doubleValuePointer;

	Serial.printf("    %s [%s]: ", item->prompt, item->formName);

	switch (item->settingType)
	{

	case text:
		Serial.println((char *)item->value);
		break;

	case password:
		Serial.println((char *)item->value);
//		Serial.println("******");
		break;

	case integerValue:
		intValuePointer = (int*)item->value;
		Serial.println(*intValuePointer);
		break;

	case doubleValue:
		doubleValuePointer = (double *)item->value;
		Serial.println(*doubleValuePointer);
		break;

	case onOff:
		boolValuePointer = (boolean*)item->value;
		if (*boolValuePointer)
		{
			Serial.println("on");
		}
		else
		{
			Serial.println("off");
		}
		break;

	case yesNo:
		boolValuePointer = (boolean*)item->value;
		if (*boolValuePointer)
		{
			Serial.println("yes");
		}
		else
		{
			Serial.println("no");
		}
		break;
	}
}


void appendSettingJSON(SettingItem * item, char * jsonBuffer, int bufferLength)
{


	int * intValuePointer;
	boolean * boolValuePointer;
	double * doubleValuePointer;

	snprintf(jsonBuffer, bufferLength,
		"%s,\"%s\":",
		jsonBuffer,
		item->formName);

	switch (item->settingType)
	{

	case text:
		snprintf(jsonBuffer, bufferLength,
			"%s\"%s\"",
			jsonBuffer,
			(char *)item->value);
		break;

	case password:
		Serial.println("******");
		snprintf(jsonBuffer, bufferLength,
			"%s\"******\"",
			jsonBuffer);
		break;

	case integerValue:
		intValuePointer = (int*)item->value;
		Serial.println(*intValuePointer);
		break;

	case doubleValue:
		doubleValuePointer = (double *)item->value;
		Serial.println(*doubleValuePointer);
		break;

	case onOff:
		boolValuePointer = (boolean*)item->value;
		if (*boolValuePointer)
		{
			Serial.println("on");
		}
		else
		{
			Serial.println("off");
		}
		break;

	case yesNo:
		boolValuePointer = (boolean*)item->value;
		if (*boolValuePointer)
		{
			Serial.println("yes");
		}
		else
		{
			Serial.println("no");
		}
		break;
	}
}


void resetSetting(SettingItem * setting)
{
	setting->setDefault(setting->value);
}

void resetSettingCollection(SettingItemCollection * settingCollection)
{
	for (int settingNo = 0; settingNo < settingCollection->noOfSettings; settingNo++)
	{
		resetSetting(&settingCollection->settings[settingNo]);
	}
}

void resetSettings()
{
	settings.majorVersion = MAJOR_VERSION;
	settings.minorVersion = MINOR_VERSION;

	for (int collectionNo = 0; collectionNo < sizeof(allSettings) / sizeof(SettingItemCollection); collectionNo++)
	{
		resetSettingCollection(&allSettings[collectionNo]);
	}

	settings.checkByte1 = CHECK_BYTE_O1;
	settings.checkByte2 = CHECK_BYTE_O2;
}

void PrintSettingCollection(SettingItemCollection settingCollection)
{
	Serial.printf("\n%s\n", settingCollection.collectionName);
	for (int settingNo = 0; settingNo < settingCollection.noOfSettings; settingNo++)
	{
		printSetting(&settingCollection.settings[settingNo]);
	}
}


void PrintAllSettings()
{
	// Start the search at setting collection 1 so that the quick settings are not printed
	for (int collectionNo = 1; collectionNo < sizeof(allSettings) / sizeof(SettingItemCollection); collectionNo++)
	{
		PrintSettingCollection(allSettings[collectionNo]);
	}
}


void writeBytesToEEPROM(byte * bytesToStore, int address, int length)
{
	int endAddress = address + length;
	for (int i = address; i < endAddress; i++)
	{
		EEPROM.write(i, *bytesToStore);
		bytesToStore++;
	}
	EEPROM.commit();
}

void readBytesFromEEPROM(byte * destination, int address, int length)
{
	int endAddress = address + length;

	for (int i = address; i < endAddress; i++)
	{
		*destination = EEPROM.read(i);

		destination++;
	}
}

void saveSettings()
{
	int addr = SETTINGS_EEPROM_OFFSET;

	byte * settingPtr = (byte *)&settings;

	writeBytesToEEPROM(settingPtr, SETTINGS_EEPROM_OFFSET, sizeof(Device_Settings));
}

void loadSettings()
{
	byte * settingPtr = (byte *)&settings;
	readBytesFromEEPROM(settingPtr, SETTINGS_EEPROM_OFFSET, sizeof(Device_Settings));
}

#define CHECK_BYTE_O1 0xAA
#define CHECK_BYTE_O2 0x55

boolean validStoredSettings()
{
	return (settings.checkByte1 == CHECK_BYTE_O1 && settings.checkByte2 == CHECK_BYTE_O2);
}


boolean matchSettingCollectionName(SettingItemCollection * settingCollection, const char * name)
{
	int settingNameLength = strlen(settingCollection->collectionName);

	for (int i = 0; i < settingNameLength; i++)
	{
		if (tolower(name[i] != settingCollection->collectionName[i]))
			return false;
	}

	// reached the end of the name, character that follows should be either zero (end of the string)
	// or = (we are assigning a value to the setting)

	if (name[settingNameLength] == 0)
		return true;

	return false;
}

SettingItemCollection * findSettingItemCollectionByName(const char * name)
{
	for (int collectionNo = 0; collectionNo < sizeof(allSettings) / sizeof(SettingItemCollection); collectionNo++)
	{
		if (matchSettingCollectionName(&allSettings[collectionNo], name))
		{
			return &allSettings[collectionNo];
		}
	}
	return NULL;
}

boolean matchSettingName(SettingItem * setting, char * name)
{
	int settingNameLength = strlen(setting->formName);

	for (int i = 0; i < settingNameLength; i++)
	{
		if (tolower(name[i] != setting->formName[i]))
			return false;
	}

	// reached the end of the name, character that follows should be either zero (end of the string)
	// or = (we are assigning a value to the setting)

	if (name[settingNameLength] == 0)
		return true;

	if (name[settingNameLength] == '=')
		return true;

	return false;
}

SettingItem * findSettingByNameInCollection(SettingItemCollection settingCollection, char * name)
{
	for (int settingNo = 0; settingNo < settingCollection.noOfSettings; settingNo++)
	{
		if (matchSettingName(&settingCollection.settings[settingNo], name))
			return &settingCollection.settings[settingNo];
	}
	return NULL;
}

SettingItem * findSettingByName(char * name)
{
	for (int collectionNo = 0; collectionNo < sizeof(allSettings) / sizeof(SettingItemCollection); collectionNo++)
	{
		SettingItem * result;
		result = findSettingByNameInCollection(allSettings[collectionNo], name);
		if (result != NULL)
			return result;
	}
	return NULL;
}

struct AllSystemSettings allSystemSettings = {
	allSettings,
	sizeof(allSettings) / sizeof(SettingItemCollection)
};

AllSystemSettings * getAllSystemSettings()
{
	return &allSystemSettings;
}

processSettingCommandResult processSettingCommand(char * command)
{
	SettingItem * setting = findSettingByName(command);

	if (setting != NULL)
	{
		// found a setting - get the length of the setting name:
		int settingNameLength = strlen(setting->formName);

		if (command[settingNameLength] == 0)
		{
			// Settting is on it's own on the line
			// Just print the value 
			printSetting(setting);
			return displayedOK;
		}

		if (command[settingNameLength] == '=')
		{
			// Setting is being assigned a value
			// move down the input to the new value
			char * startOfSettingInfo = command + settingNameLength + 1;
			if (setting->validateValue(setting->value, startOfSettingInfo))
			{
				saveSettings();
				return setOK;
			}
			return settingValueInvalid;
		}
	}
	return settingNotFound;
}

void testSettingsStorage()
{
	Serial.println("Testing Settings Storage");
	Serial.println("Resetting Settings");
	resetSettings();
	PrintAllSettings();
	Serial.println("Storing Settings");
	saveSettings();
	Serial.println("Loading Settings");
	loadSettings();
	PrintAllSettings();
	if (validStoredSettings())
		Serial.println("Settings storage restored OK");
	else
		Serial.println("Something wrong with setting storage");
}

void setupSettings()
{
	EEPROM.begin(EEPROM_SIZE);

	loadSettings();

	if (!validStoredSettings())
	{
		Serial.println("Stored settings reset");
		resetSettings();
		saveSettings();
	}
}

void createSettingsJson(char * jsonBuffer, int length)
{
	snprintf(jsonBuffer, length, "{ \"dev\":\"%s\",\"plat\":\WEMOS\"",
		settings.deviceName);
}

void testFindSettingByName()
{
	resetSettings();

	SettingItem * result = findSettingByName("pixelControlPinNo");
	if (result != NULL)
		Serial.println("Setting find works OK");
	else
		Serial.println("Setting find failed");

	result = findSettingByName("xpixelControlPinNo");
	if (result == NULL)
		Serial.println("Setting not find works OK");
	else
		Serial.println("Setting not find failed");

	result = findSettingByName("pixelControlPinNox");
	if (result == NULL)
		Serial.println("Setting not find works OK");
	else
		Serial.println("Setting not find failed");

	if (processSettingCommand("fred"))
		Serial.println("Should not have processed setting fred");
	else
		Serial.println("Setting not found OK");

	if (processSettingCommand("devname"))
		Serial.println("Found setting devname - should have printed it");
	else
		Serial.println("Setting devname not found");

	if (processSettingCommand("devname=test"))
		Serial.println("Found setting devname - should have changed it to test");
	else
		Serial.println("Setting devname not found");

	if (processSettingCommand("devname"))
		Serial.println("Found setting devname - should have printed it");
	else
		Serial.println("Setting devname not found");
}