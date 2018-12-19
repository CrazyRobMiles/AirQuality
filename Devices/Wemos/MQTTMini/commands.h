#pragma once

#define EEPROM_SIZE 2000
#define SETTINGS_EEPROM_OFFSET 0

#define WORKED_OK 0
#define NUMERIC_VALUE_NOT_AN_INTEGER 1
#define NUMERIC_VALUE_BELOW_MINIMUM 2
#define NUMERIC_VALUE_ABOVE_MAXIMUM 3
#define INVALID_HEX_DIGIT_IN_VALUE 4
#define INCOMING_HEX_VALUE_TOO_BIG_FOR_BUFFER 5
#define VALUE_MISSING_OR_NOT_A_STRING 6
#define INCOMING_HEX_VALUE_IS_THE_WRONG_LENGTH 7
#define INVALID_OR_MISSING_TARGET_IN_RECEIVED_COMMAND 8
#define COMMAND_FOR_DIFFERENT_TARGET 9
#define INVALID_OR_MISSING_DEVICE_NAME 10
#define INVALID_DEVICE_NAME_FOR_MQTT_ON 11
#define STRING_VALUE_MISSING_OR_NOT_STRING 13
#define STRING_VALUE_TOO_LONG 14
#define INVALID_LORA_ACCESS_SETTING 15
#define INVALID_COMMAND_NAME 16
#define JSON_COMMAND_COULD_NOT_BE_PARSED 17
#define JSON_COMMAND_MISSING_VERSION 18
#define JSON_COMMAND_MISSING_COMMAND 19
#define JSON_COMMAND_MISSING_OPTION 20
#define INVALID_MQTT_STATUS_SETTING 21
#define INVALID_LORA_STATUS_SETTING 22
#define MISSING_WIFI_SETTING_NUMBER 23
#define INVALID_WIFI_SETTING_NUMBER 24
#define INVALID_COLOUR_NAME 25

#define REPLY_ELEMENT_SIZE 100
#define COMMAND_REPLY_BUFFER_SIZE 240

char command_reply_buffer[COMMAND_REPLY_BUFFER_SIZE];

void build_command_reply(int errorNo, JsonObject& root, char * resultBuffer)
{
	char replyBuffer[REPLY_ELEMENT_SIZE];

	const char * sequence = root["seq"];

	if (sequence)
	{
		// Got a sequence number in the command - must return the same number
		// so that the sender can identify the command that was sent
		int sequenceNo = root["seq"];
		sprintf(replyBuffer, "\"error\":%d,\"seq\":%d", errorNo, sequenceNo);
	}
	else
	{
		sprintf(replyBuffer, "\"error\":%d", errorNo);
	}
	strcat(resultBuffer, replyBuffer);
}

void build_text_value_command_reply(int errorNo, char * result, JsonObject& root, char * resultBuffer)
{
	char replyBuffer[REPLY_ELEMENT_SIZE];

	const char * sequence = root["seq"];

	if (sequence)
	{
		// Got a sequence number in the command - must return the same number
		// so that the sender can identify the command that was sent
		int sequenceNo = root["seq"];
		sprintf(replyBuffer, "\"val\":\"%s\",\"error\":%d,\"seq\":%d", result, errorNo, sequenceNo);
	}
	else
	{
		sprintf(replyBuffer, "\"val\":\"%s\",\"error\":%d", result, errorNo);
	}

	strcat(resultBuffer, replyBuffer);
}

void build_number_value_command_reply(int errorNo, int result, JsonObject& root, char * resultBuffer)
{
	char replyBuffer[REPLY_ELEMENT_SIZE];

	const char * sequence = root["seq"];

	if (sequence)
	{
		// Got a sequence number in the command - must return the same number
		// so that the sender can identify the command that was sent
		int sequenceNo = root["seq"];
		sprintf(replyBuffer, "\"val\":%d,\"error\":%d,\"seq\":%d", result, errorNo, sequenceNo);
	}
	else
	{
		sprintf(replyBuffer, "\"val\":%d,\"error\":%d", result, errorNo);
	}

	strcat(resultBuffer, replyBuffer);
}

void dump_hex(u1_t * pos, int length)
{
	while (length > 0)
	{
		// handle leading zeroes
		if (*pos < 0x10) {
			TRACE("0");
		}
		TRACE_HEX(*pos);
		pos++;
		length--;
	}
	TRACELN();
}

char hex_digit(int val)
{
	if (val < 10)
	{
		return '0' + val;
	}
	else
	{
		return 'A' + (val - 10);
	}
}

void dump_hex_string(char * dest, u1_t * pos, int length)
{
	while (length > 0)
	{
		// handle leading zeroes

		*dest = hex_digit(*pos / 16);
		dest++;
		*dest = hex_digit(*pos % 16);
		dest++;
		pos++;
		length--;
	}
	*dest = 0;
}

void dump_unsigned_long(char * dest, u4_t value)
{
	// Write backwards to put least significant values in
	// the right place

	// move to the end of the string
	int pos = 8;
	// put the terminator in position
	dest[pos] = 0;
	pos--;

	while (pos > 0)
	{
		byte b = value & 0xff;
		dest[pos] = hex_digit(b % 16);
		pos--;
		dest[pos] = hex_digit(b / 16);
		pos--;
		value = value >> 8;
	}
}

void dump_settings()
{
	TRACE("Version: "); TRACELN(settings.version);
	TRACE("Device name: "); TRACELN(settings.deviceNane);
	TRACE("MQTT server: "); TRACELN(settings.mqttServer);
	TRACE("MQTT port: "); TRACELN(settings.mqttPort);
	TRACE("MQTT user: "); TRACELN(settings.mqttUser);
	TRACE("MQTT password: "); TRACELN(settings.mqttPassword);
	TRACE("MQTT device name: "); TRACELN(settings.mqttName);
	TRACE("MQTT publish topic: "); TRACELN(settings.mqttPublishTopic);
	TRACE("MQTT subscribe topic: "); TRACELN(settings.mqttSubscribeTopic);
	TRACE("MQTT seconds per update: "); TRACELN(settings.seconds_per_mqtt_update);
	TRACE("MQTT enabled: "); TRACELN(settings.mqtt_enabled);
}

void writeBytesToEEPROM ( byte * bytesToStore, int address, int length)
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


void save_settings()
{
	int addr = SETTINGS_EEPROM_OFFSET;

	byte * settingPtr = (byte *)&settings;

	writeBytesToEEPROM(settingPtr, SETTINGS_EEPROM_OFFSET, sizeof(Device_Settings));

	dump_settings();
}

void load_settings()
{
	byte * settingPtr = (byte *)&settings;
	readBytesFromEEPROM(settingPtr, SETTINGS_EEPROM_OFFSET, sizeof(Device_Settings));
}

int decodeStringValue(char * dest, JsonObject& root, const char * valName, int maxLength)
{
	TRACE("Decoding string value: ");
	TRACELN(valName);

	if (!root[valName].is<char*>())
	{
		TRACELN("Value is missing or not a string");
		return STRING_VALUE_MISSING_OR_NOT_STRING;
	}
	else
	{
		String newVal = root[valName];
		if (newVal.length() > maxLength)
		{
			TRACELN("Value is too long");
			return STRING_VALUE_TOO_LONG;
		}
		newVal.toCharArray(dest, maxLength);
		return WORKED_OK;
	}
}

int decodeNumericValue(int * dest, JsonObject& root, const char * valName, int min, int max)
{
	TRACE("Decoding numeric value: ");
	TRACELN(valName);

	if (!root[valName].is<int>())
	{
		TRACELN("Value is missing or not an integer");
		return NUMERIC_VALUE_NOT_AN_INTEGER;
	}
	else
	{
		int newGapVal = root[valName];
		if (newGapVal < min)
		{
			TRACELN("Value below minimum");
			return NUMERIC_VALUE_BELOW_MINIMUM;
		}
		if (newGapVal > max)
		{
			TRACELN("Value above maximum");
			return NUMERIC_VALUE_ABOVE_MAXIMUM;
		}
		*dest = newGapVal;
		return WORKED_OK;
	}
}

int hexFromChar(char c, int * dest)
{
	if (c >= '0' && c <= '9')
	{
		*dest = (int)(c - '0');
		return WORKED_OK;
	}
	else
	{
		if (c >= 'A' && c <= 'F')
		{
			*dest = (int)(c - 'A' + 10);
			return WORKED_OK;
		}
		else
		{
			if (c >= 'a' && c <= 'f')
			{
				*dest = (int)(c - 'a' + 10);
				return WORKED_OK;
			}
		}
	}
	return INVALID_HEX_DIGIT_IN_VALUE;
}

#define MAX_DECODE_BUFFER_LENGTH 20

int decodeHexValueIntoBytes(u1_t * dest, JsonObject& root, const char * valName, int length)
{
	TRACE("Decoding array of bytes value: ");
	TRACELN(valName);

	if (!root[valName].is<char*>())
	{
		TRACELN("Value is missing or not a string");
		return VALUE_MISSING_OR_NOT_A_STRING;
	}

	if (length > MAX_DECODE_BUFFER_LENGTH)
	{
		TRACELN("Incoming hex value will not fit in the buffer");
		return INCOMING_HEX_VALUE_TOO_BIG_FOR_BUFFER;
	}

	String newVal = root[valName];
	// Each hex value is in two bytes - make sure the incoming text is the right length

	if (newVal.length() != length * 2)
	{
		TRACELN("Incoming hex value is the wrong length");
		return INCOMING_HEX_VALUE_IS_THE_WRONG_LENGTH;
	}

	int pos = 0;

	u1_t buffer[MAX_DECODE_BUFFER_LENGTH];
	u1_t * bpos = buffer;

	while (pos < newVal.length())
	{
		int d1, d2, reply;

		reply = hexFromChar(newVal[pos], &d1);
		if (reply != WORKED_OK)
			return reply;
		pos++;
		reply = hexFromChar(newVal[pos], &d2);
		if (reply != WORKED_OK)
			return reply;
		pos++;

		*bpos = (u1_t)(d1 * 16 + d2);
		bpos++;
	}

	// If we get here the buffer has been filled OK

	memcpy_P(dest, buffer, length);
	return WORKED_OK;
}

int decodeHexValueIntoUnsignedLong(u4_t * dest, JsonObject& root, const char * valName)
{
	TRACE("Decoding unsigned long value: ");
	TRACELN(valName);

	if (!root[valName].is<char*>())
	{
		TRACELN("Value is missing or not a string");
		return VALUE_MISSING_OR_NOT_A_STRING;
	}
	// Each hex value is in two bytes - make sure the incoming text is the right length

	String newVal = root[valName];
	if (newVal.length() != 8)
	{
		TRACELN("Incoming hex value is the wrong length");
		return INCOMING_HEX_VALUE_IS_THE_WRONG_LENGTH;
	}

	int pos = 0;

	u4_t result = 0;

	while (pos < newVal.length())
	{
		int d1, d2, reply;

		reply = hexFromChar(newVal[pos], &d1);
		if (reply != WORKED_OK)
			return reply;
		pos++;
		reply = hexFromChar(newVal[pos], &d2);
		if (reply != WORKED_OK)
			return reply;
		pos++;

		u4_t v = d1 * 16 + d2;
		result = result * 256 + v;
	}

	// If we get here the value has been received OK

	*dest = result;
	return WORKED_OK;
}

int checkTargetDeviceName(JsonObject& root)
{
	const char * target = root["t"];

	if (!target)
	{
		TRACELN("Invalid or missing target in received command");
		return INVALID_OR_MISSING_TARGET_IN_RECEIVED_COMMAND;
	}

	if (strcasecmp(target, settings.deviceNane))
	{
		TRACE("Command for different target: ");
		TRACELN(target);
		return COMMAND_FOR_DIFFERENT_TARGET;
	}

	TRACELN("Doing a command for this device");
	return WORKED_OK;
}

// request: {"v":1, "t" : "Sensor01", "c" : "mqtt", "o" : "send"}
void do_send_mqtt(JsonObject& root, char * resultBuffer)
{
	int reply = checkTargetDeviceName(root);

	if (reply == WORKED_OK)
	{
		pub_mqtt_force_send = true;
	}
	build_command_reply(reply, root, resultBuffer);
}

// {"v":1, "t" : "Sensor01", "c" : "mqtt", "o" : "state", "v" : "on"}
// {"v":1, "t" : "Sensor01", "c" : "mqtt", "o" : "state", "v" : "off"}
void do_mqtt_state(JsonObject& root, char * resultBuffer)
{
	int reply = checkTargetDeviceName(root);

	if (reply == WORKED_OK)
	{
		const char * option = root["val"];
		if (!option)
		{
			// no option - just a status request
			if (settings.mqtt_enabled)
			{
				build_text_value_command_reply(WORKED_OK, "on", root, resultBuffer);
			}
			else
			{
				build_text_value_command_reply(WORKED_OK, "off", root, resultBuffer);
			}
			return;
		}

		if (!root["val"].is<char*>())
		{
			TRACELN("Value is missing or not a string");
			reply = INVALID_MQTT_STATUS_SETTING;
		}
		else
		{
			const char * option = root["val"];
			if (strcasecmp(option, "on") == 0)
			{
				settings.mqtt_enabled = true;
			}
			else
			{
				if (strcasecmp(option, "off") == 0)
				{
					settings.mqtt_enabled = false;
				}
				else
				{
					reply = INVALID_MQTT_STATUS_SETTING;
				}
			}
		}
	}

	if (reply == WORKED_OK)
		save_settings();

	build_command_reply(reply, root, resultBuffer);
}

#define MIN_MQTT_GAP 1
#define MAX_MQTT_GAP 30000

// {"v":1, "t" : "sensor01", "c" : "mqtt", "o" : "gap", "val":20}
void do_mqtt_gap(JsonObject& root, char * resultBuffer)
{
	int reply = checkTargetDeviceName(root);

	if (reply == WORKED_OK)
	{
		const char * option = root["val"];

		if (!option)
		{
			build_number_value_command_reply(WORKED_OK, settings.seconds_per_mqtt_update, root, resultBuffer);
			return;
		}

		reply = decodeNumericValue(&settings.seconds_per_mqtt_update, root, "val", MIN_MQTT_GAP, MAX_MQTT_GAP);

		if (reply == WORKED_OK)
		{
			save_settings();
		}
	}

	setup_timing();
	build_command_reply(reply, root, resultBuffer);
}

#define MIN_MQTT_RETRY 1
#define MAX_MQTT_RETRY 30000

// {"v":1, "t" : "sensor01", "c" : "mqtt", "o" : "retry", "val":20}
void do_mqtt_retry(JsonObject& root, char * resultBuffer)
{
	int reply = checkTargetDeviceName(root);

	if (reply == WORKED_OK)
	{
		const char * option = root["val"];

		if (!option)
		{
			build_number_value_command_reply(WORKED_OK, settings.seconds_per_mqtt_retry, root, resultBuffer);
			return;
		}

		reply = decodeNumericValue(&settings.seconds_per_mqtt_retry, root, "val", MIN_MQTT_RETRY, MAX_MQTT_RETRY);

		if (reply == WORKED_OK)
		{
			save_settings();
		}
	}

	build_command_reply(reply, root, resultBuffer);
}


// {"v":1, "t" : "sensor01", "c" : "mqtt", "o" : "publish", "val":"sensor01/data" }
void do_mqtt_publish_location(JsonObject& root, char * resultBuffer)
{
	int reply = checkTargetDeviceName(root);

	if (reply == WORKED_OK)
	{
		const char * option = root["val"];

		if (!option)
		{
			build_text_value_command_reply(WORKED_OK, settings.mqttPublishTopic, root, resultBuffer);
			return;
		}

		reply = decodeStringValue(settings.mqttPublishTopic, root, "val", MQTT_PUBLISH_TOPIC_LENGTH - 1);
		if (reply == WORKED_OK)
		{
			save_settings();
		}
	}

	build_command_reply(reply, root, resultBuffer);
}

// {"v":1, "t" : "sensor01", "c" : "mqtt", "o" : "subscribe", "val":"sensor01/commands" }
void do_mqtt_subscribe_location(JsonObject& root, char * resultBuffer)
{
	int reply = checkTargetDeviceName(root);

	if (reply == WORKED_OK)
	{
		const char * option = root["val"];

		if (!option)
		{
			build_text_value_command_reply(WORKED_OK, settings.mqttSubscribeTopic, root, resultBuffer);
			return;
		}

		reply = decodeStringValue(settings.mqttSubscribeTopic, root, "val", MQTT_SUBSCRIBE_TOPIC_LENGTH - 1);
		if (reply == WORKED_OK)
		{
			save_settings();
		}
	}

	build_command_reply(reply, root, resultBuffer);
}

// {"v":1, "t" : "sensor01", "c" : "mqtt", "o" : "id", "val":"sensor01" }
void do_mqtt_device_id(JsonObject& root, char * resultBuffer)
{
	int reply = checkTargetDeviceName(root);

	if (reply == WORKED_OK)
	{
		const char * option = root["val"];

		if (!option)
		{
			build_text_value_command_reply(WORKED_OK, settings.mqttName, root, resultBuffer);
			return;
		}

		reply = decodeStringValue(settings.mqttName, root, "val", MQTT_DEVICE_NAME_LENGTH - 1);
		if (reply == WORKED_OK)
		{
			save_settings();
		}
	}

	build_command_reply(reply, root, resultBuffer);
}


// {"v":1, "t" : "sensor01", "c" : "mqtt", "o" : "host", "val":"mqtt.connectedhumber.org" }
void do_mqtt_host(JsonObject& root, char * resultBuffer)
{
	int reply = checkTargetDeviceName(root);

	if (reply == WORKED_OK)
	{
		const char * option = root["val"];

		if (!option)
		{
			build_text_value_command_reply(WORKED_OK, settings.mqttServer, root, resultBuffer);
			return;
		}

		reply = decodeStringValue(settings.mqttServer, root, "val", MQTT_SERVER_NAME_LENGTH - 1);
		if (reply == WORKED_OK)
		{
			save_settings();
		}
	}

	build_command_reply(reply, root, resultBuffer);
}

// {"v":1, "t" : "sensor01", "c" : "mqtt", "o" : "user", "val":"username" }
void do_mqtt_user(JsonObject& root, char * resultBuffer)
{
	int reply = checkTargetDeviceName(root);

	if (reply == WORKED_OK)
	{
		const char * option = root["val"];

		if (!option)
		{
			build_text_value_command_reply(WORKED_OK, settings.mqttUser, root, resultBuffer);
			return;
		}

		reply = decodeStringValue(settings.mqttPassword, root, "val", MQTT_USER_NAME_LENGTH - 1);
		if (reply == WORKED_OK)
		{
			save_settings();
		}
	}

	build_command_reply(reply, root, resultBuffer);
}

// {"v":1, "t" : "sensor01", "c" : "mqtt", "o" : "pwd", "val":"123456" }
void do_mqtt_password(JsonObject& root, char * resultBuffer)
{
	int reply = checkTargetDeviceName(root);

	if (reply == WORKED_OK)
	{
		const char * option = root["val"];

		if (!option)
		{
			build_text_value_command_reply(WORKED_OK, settings.mqttPassword, root, resultBuffer);
			return;
		}

		reply = decodeStringValue(settings.mqttPassword, root, "val", MQTT_PASSWORD_LENGTH - 1);
		if (reply == WORKED_OK)
		{
			save_settings();
		}
	}

	build_command_reply(reply, root, resultBuffer);
}

// {"v":1, "t" : "sensor01", "c" : "mqtt", "o" : "port", "val":1883 }
void do_mqtt_port(JsonObject& root, char * resultBuffer)
{
	int reply = checkTargetDeviceName(root);

	if (reply == WORKED_OK)
	{
		const char * option = root["val"];

		if (!option)
		{
			build_number_value_command_reply(WORKED_OK, settings.mqttPort, root, resultBuffer);
			return;
		}

		reply = decodeNumericValue(&settings.mqttPort, root, "val", 0, 2047);
		if (reply == WORKED_OK)
		{
			save_settings();
		}
	}

	build_command_reply(reply, root, resultBuffer);
}

StaticJsonBuffer<300> jsonBuffer;

struct OptionDecodeItems
{
	const char * optionName;
	void(*optionAction) (JsonObject& root, char * resultBuffer);
};

OptionDecodeItems mqttOptionDecodeItems[] = {
	{"state", do_mqtt_state},
	{"send", do_send_mqtt},
    {"gap", do_mqtt_gap},
	{"retry", do_mqtt_retry},
	{"publish", do_mqtt_publish_location},
	{"subscribe", do_mqtt_subscribe_location},
	{"id", do_mqtt_device_id},
	{"user", do_mqtt_user},
	{"pwd", do_mqtt_password},
	{"host", do_mqtt_host},
	{"port", do_mqtt_port}
};


// forward declaration - function in WiFiConnection.h

void startWifi();

// {"v":1, "t" : "Sensor01", "c" : "wifi", "o" : "on"}
void do_wifi_on(JsonObject& root, char * resultBuffer)
{
	int reply = checkTargetDeviceName(root);

	if (reply == WORKED_OK)
	{
		TRACELN("Starting WiFi");
		startWifi();
	}
	build_command_reply(reply, root, resultBuffer);
}

// {"v":1, "t" : "Sensor01", "c" : "wifi", "o" : "off"}
void do_wifi_off(JsonObject& root, char * resultBuffer)
{
	int reply = checkTargetDeviceName(root);

	if (reply == WORKED_OK)
	{
		TRACELN("Stopping WiFi");
		// not sure how to do this just yet...
	}
	build_command_reply(reply, root, resultBuffer);
}

// {"v":1, "t" : "sensor01", "c" : "wifi",  "o" : "ssid", "set":0, "val":"ssid"}
void do_wifi_ssid(JsonObject& root, char * resultBuffer)
{
	int reply = checkTargetDeviceName(root);

	if (reply == WORKED_OK)
	{
		if (!root["set"].is<int>())
		{
			TRACELN("Missing WiFi setting number");
			reply = MISSING_WIFI_SETTING_NUMBER;
		}
		else
		{
			int settingNo = root["set"];
			if (settingNo < 0 || settingNo >= NO_OF_WIFI_SETTINGS)
			{
				reply = INVALID_WIFI_SETTING_NUMBER;
			}
			else
			{
				const char * option = root["val"];

				if (!option)
				{
					build_text_value_command_reply(WORKED_OK, settings.wifiSettings[settingNo].wifiSsid,
						root, resultBuffer);
					return;
				}

				reply = decodeStringValue(settings.wifiSettings[settingNo].wifiSsid, root,
					"val", WIFI_PASSWORD_LENGTH - 1);
			}

			if (reply == WORKED_OK)
			{
				save_settings();
			}
		}
	}

	build_command_reply(reply, root, resultBuffer);
}

// {"v":1, "t" : "sensor01", "c" : "wifi", "o" : "pwd", "set":0, "val":"password"}
void do_wifi_password(JsonObject& root, char * resultBuffer)
{
	int reply = checkTargetDeviceName(root);

	if (reply == WORKED_OK)
	{
		if (!root["set"].is<int>())
		{
			TRACELN("Missing WiFi setting number");
			reply = MISSING_WIFI_SETTING_NUMBER;
		}
		else
		{
			int settingNo = root["set"];
			if (settingNo < 0 || settingNo >= NO_OF_WIFI_SETTINGS)
			{
				reply = INVALID_WIFI_SETTING_NUMBER;
			}
			else
			{
				reply = decodeStringValue(settings.wifiSettings[settingNo].wifiPassword, root, "val", WIFI_PASSWORD_LENGTH - 1);
			}

			if (reply == WORKED_OK)
			{
				save_settings();
			}
		}
	}

	build_command_reply(reply, root, resultBuffer);
}
OptionDecodeItems WiFiOptionDecodeItems[] = {
	{"on", do_wifi_on},
	{"off", do_wifi_off},
	{"ssid", do_wifi_ssid},
	{"pwd", do_wifi_password}
};


// {"v":1, "c" : "node", "o" : "ver"}
void do_node_version(JsonObject& root, char * resultBuffer)
{
	TRACELN("Getting version");
	int length = sprintf(resultBuffer, "{\"version\":%d,", settings.version);
	build_command_reply(WORKED_OK, root, resultBuffer + length);
}

// {"v":1, "c" : "node", "o" : "getdevname"}
void do_get_device_name(JsonObject& root, char * resultBuffer)
{
	TRACELN("Getting device name");

	int length = sprintf(resultBuffer, "{\"nodename\":\"%s\",", settings.deviceNane);
	build_command_reply(WORKED_OK, root, resultBuffer + length);
}

// {"v":1, "t" : "sensor01", "c" : "node", "o" : "devname", "val":"sensor01"}
void do_device_name(JsonObject& root, char * resultBuffer)
{
	int reply = checkTargetDeviceName(root);

	if (reply == WORKED_OK)
	{
		const char * option = root["val"];

		if (!option)
		{
			build_text_value_command_reply(WORKED_OK, settings.deviceNane, root, resultBuffer);
			return;
		}

		reply = decodeStringValue(settings.deviceNane, root, "val", DEVICE_NAME_LENGTH - 1);
		if (reply == WORKED_OK)
		{
			save_settings();
		}
	}

	build_command_reply(reply, root, resultBuffer);
}

// {"v":1, "t" : "sensor01", "c" : "node", "o" : "reset"}
void do_reset(JsonObject& root, char * resultBuffer)
{
	int reply = checkTargetDeviceName(root);

	if (reply == WORKED_OK)
	{
		ESP.restart();
	}
	build_command_reply(reply, root, resultBuffer);
}

#define COLOUR_NAME_LENGTH 10

void updatePixelColor(float r, float g, float b)
{
	settings.pixel_red = r;
	settings.pixel_green = g;
	settings.pixel_blue = b;
	save_settings();
}

struct colourDecode {
	char * colourName;
	float red;
	float green;
	float blue;
};

struct colourDecode colourDecoder[] = {
	{"red", 1.0, 0, 0},
	{"green", 0,1.0,0},
	{"blue", 0,1.0,0},
	{"cyan", 0,1.0,1.0},
	{"magenta", 1.0,0,1.0},
	{"yellow", 1.0,1.0,0},
	{"white", 1.0,1.0,1.0},
	{"black", 0,0,0},
};


bool setDecodedColor(char * name)
{
	for (int i = 0; i < sizeof(colourDecoder) / sizeof(colourDecode); i++)
	{
		if (strcasecmp(name, colourDecoder[i].colourName) == 0)
		{
			settings.pixel_red = colourDecoder[i].red;
			settings.pixel_green = colourDecoder[i].green;
			settings.pixel_blue = colourDecoder[i].blue;
			return true;
		}
	}
	return false;
}


// {"v":1, "t" : "sensor01", "c" : "node", "o" : "pixel", "val":"red"}
void do_pixel_colour(JsonObject& root, char * resultBuffer)
{
	int reply = checkTargetDeviceName(root);

	if (reply == WORKED_OK)
	{
		char colourName [COLOUR_NAME_LENGTH];
		reply = decodeStringValue(colourName, root, "val", COLOUR_NAME_LENGTH - 1);
		if (setDecodedColor(colourName))
		{
			save_settings();
		}
		else
		{
			reply = INVALID_COLOUR_NAME;
		}
	}

	build_command_reply(reply, root, resultBuffer);
}

// {"v":1, "t" : "sensor01", "c" : "node", "o" : "pixels", "val":1}
void do_noOfPixels(JsonObject& root, char * resultBuffer)
{
	int reply = checkTargetDeviceName(root);

	if (reply == WORKED_OK)
	{
		const char * option = root["val"];

		if (!option)
		{
			build_number_value_command_reply(WORKED_OK, settings.noOfPixels, root, resultBuffer);
			return;
		}

		reply = decodeNumericValue(&settings.noOfPixels, root, "val", 0, MAX_NO_OF_PIXELS);

		if (reply == WORKED_OK)
		{
			save_settings();
		}
	}

	build_command_reply(reply, root, resultBuffer);
}


// {"v":1, "t" : "sensor01", "c" : "node", "o" : "airqlow", "val":1}
void do_airqLowLimit(JsonObject& root, char * resultBuffer)
{
	int reply = checkTargetDeviceName(root);

	if (reply == WORKED_OK)
	{
		const char * option = root["val"];

		if (!option)
		{
			build_number_value_command_reply(WORKED_OK, settings.airqLowLimit, root, resultBuffer);
			return;
		}

		reply = decodeNumericValue(&settings.airqLowLimit, root, "val", 0, settings.airqLowWarnLimit);

		if (reply == WORKED_OK)
		{
			save_settings();
		}
	}

	build_command_reply(reply, root, resultBuffer);
}

// {"v":1, "t" : "sensor01", "c" : "node", "o" : "airqlowwarn", "val":2}
void do_airqLowWarnLimit(JsonObject& root, char * resultBuffer)
{
	int reply = checkTargetDeviceName(root);

	if (reply == WORKED_OK)
	{
		const char * option = root["val"];

		if (!option)
		{
			build_number_value_command_reply(WORKED_OK, settings.airqLowWarnLimit, root, resultBuffer);
			return;
		}

		reply = decodeNumericValue(&settings.airqLowWarnLimit, root, "val", settings.airqLowLimit, settings.airqMidWarnLimit);

		if (reply == WORKED_OK)
		{
			save_settings();
		}
	}

	build_command_reply(reply, root, resultBuffer);
}


// {"v":1, "t" : "sensor01", "c" : "node", "o" : "airqmidwarn", "val":2}
void do_airqMidWarnLimit(JsonObject& root, char * resultBuffer)
{
	int reply = checkTargetDeviceName(root);

	if (reply == WORKED_OK)
	{
		const char * option = root["val"];

		if (!option)
		{
			build_number_value_command_reply(WORKED_OK, settings.airqMidWarnLimit, root, resultBuffer);
			return;
		}

		reply = decodeNumericValue(&settings.airqMidWarnLimit, root, "val", settings.airqLowWarnLimit, settings.airqHighWarnLimit);

		if (reply == WORKED_OK)
		{
			save_settings();
		}
	}

	build_command_reply(reply, root, resultBuffer);
}

// {"v":1, "t" : "sensor01", "c" : "node", "o" : "airqhighwarn", "val":2}
void do_airqHighWarnLimit(JsonObject& root, char * resultBuffer)
{
	int reply = checkTargetDeviceName(root);

	if (reply == WORKED_OK)
	{
		const char * option = root["val"];

		if (!option)
		{
			build_number_value_command_reply(WORKED_OK, settings.airqHighWarnLimit, root, resultBuffer);
			return;
		}

		reply = decodeNumericValue(&settings.airqHighWarnLimit, root, "val", settings.airqMidWarnLimit, settings.airQHighAlertLimit);

		if (reply == WORKED_OK)
		{
			save_settings();
		}
	}

	build_command_reply(reply, root, resultBuffer);
}

//int airqLowLimit; 0 - airqLowWarnLimit
//int airqLowWarnLimit; airQLowLimit - arqMidWarnLimit
//int arqMidWarnLimit;  airqLowWarnLimit - airqHighWarnLimit
//int airqHighWarnLimit; arqMidWarnLimit - airQHighAlertLimit
//int airQHighAlertLimit; > airqHighWarnLimit



// {"v":1, "t" : "sensor01", "c" : "node", "o" : "airqhighalert", "val":2}
void do_airqHighAlertLimit(JsonObject& root, char * resultBuffer)
{
	int reply = checkTargetDeviceName(root);

	if (reply == WORKED_OK)
	{
		const char * option = root["val"];

		if (!option)
		{
			build_number_value_command_reply(WORKED_OK, settings.airQHighAlertLimit, root, resultBuffer);
			return;
		}

		reply = decodeNumericValue(&settings.airQHighAlertLimit, root, "val", settings.airQHighAlertLimit, 30000);

		if (reply == WORKED_OK)
		{
			save_settings();
		}
	}

	build_command_reply(reply, root, resultBuffer);
}


// {"v":1, "t" : "sensor01", "c" : "node", "o" : "airqsensor", "val":1}
void do_airqSensorType(JsonObject& root, char * resultBuffer)
{
	int reply = checkTargetDeviceName(root);

	if (reply == WORKED_OK)
	{
		const char * option = root["val"];

		if (!option)
		{
			build_number_value_command_reply(WORKED_OK, settings.airqSensorType, root, resultBuffer);
			return;
		}

		reply = decodeNumericValue(&settings.airqSensorType, root, "val", MIN_AIRQ_SENSOR_NO, MAX_AIRQ_SENSOR_NO);

		if (reply == WORKED_OK)
		{
			save_settings();
		}
	}

	build_command_reply(reply, root, resultBuffer);
}


OptionDecodeItems nodeOptionDecodeItems[] = {
	{"ver", do_node_version},
	{"getdevname", do_get_device_name},
	{"reset", do_reset },
	{"devname", do_device_name},
	{"pixels", do_noOfPixels },
	{"pixel", do_pixel_colour },
	{"airqlow", do_airqLowLimit},
	{"airqlowwarn", do_airqLowWarnLimit},
	{"airqmidwarn", do_airqMidWarnLimit},
	{"airqhighwarn", do_airqHighWarnLimit},
	{"airqhighalert", do_airqHighAlertLimit},
	{"airqsensor", do_airqSensorType},
};

struct CommandDecoder
{
	const char * commandName;
	OptionDecodeItems * items;
	int noOfOptions;
};


struct CommandDecoder commandDecoder[] =
{
	{"node", nodeOptionDecodeItems, sizeof(nodeOptionDecodeItems) / sizeof(OptionDecodeItems)},
	{"mqtt", mqttOptionDecodeItems, sizeof(mqttOptionDecodeItems) / sizeof(OptionDecodeItems)},
	{"wifi", WiFiOptionDecodeItems, sizeof(WiFiOptionDecodeItems) / sizeof(OptionDecodeItems)}
};

void actOnCommand(const char * command, const char * option, JsonObject& root, char * resultBuffer)
{
	bool foundCommand = false;

	for (int commandNo = 0; commandNo < sizeof(commandDecoder) / sizeof(CommandDecoder); commandNo++)
	{
		if (strcasecmp(command, commandDecoder[commandNo].commandName) == 0)
		{
			TRACE("Performing command: ");
			TRACELN(commandDecoder[commandNo].commandName);

			OptionDecodeItems * items = commandDecoder[commandNo].items;

			for (int optionNo = 0; optionNo < commandDecoder[commandNo].noOfOptions; optionNo++)
			{
				if (strcasecmp(option, items[optionNo].optionName) == 0)
				{
					TRACE("    Performing option: ");
					TRACELN(items[optionNo].optionName);
					items[optionNo].optionAction(root, command_reply_buffer);
					foundCommand = true;
				}
			}
		}
	}

	if (foundCommand)
	{
		TRACELN("Command found and performed.");
	}
	else
	{
		build_command_reply(INVALID_COMMAND_NAME, root, resultBuffer);
		TRACELN("Command not found");
	}
}


void abort_json_command(int error, JsonObject& root, void(*deliverResult) (char * resultText))
{
	build_command_reply(error, root, command_reply_buffer);
	strcat(command_reply_buffer, "}");
	deliverResult(command_reply_buffer);
}

void act_onJson_command(char * json, void(*deliverResult) (char * resultText))
{
	command_reply_buffer[0] = 0;

	strcat(command_reply_buffer, "{");

	TRACE("Received command:");
	TRACELN(json);

	// Clear any previous elements from the buffer

	jsonBuffer.clear();

	JsonObject& root = jsonBuffer.parseObject(json);

	if (!root.success())
	{
		TRACELN("JSON could not be parsed");
		abort_json_command(JSON_COMMAND_COULD_NOT_BE_PARSED, root, deliverResult);
		return;
	}

	int v = root["v"];

	if (v != settings.version)
	{
		TRACELN("Invalid or missing version in received command");
		TRACELN(v);
		abort_json_command(JSON_COMMAND_MISSING_VERSION, root, deliverResult);
		return;
	}

	const char * command = root["c"];

	if (!command)
	{
		TRACELN("Missing command");
		abort_json_command(JSON_COMMAND_MISSING_COMMAND, root, deliverResult);
		return;
	}

	TRACE("Received command: ");
	TRACELN(command);

	const char * option = root["o"];

	if (!option)
	{
		TRACELN("Missing option");
		TRACELN(v);
		abort_json_command(JSON_COMMAND_MISSING_OPTION, root, deliverResult);
		return;
	}

	TRACE("Received option: ");
	TRACELN(option);

	actOnCommand(command, option, root, command_reply_buffer);
	strcat(command_reply_buffer, "}");
	deliverResult(command_reply_buffer);
}

#define CHECK_BYTE_O1 0xAA
#define CHECK_BYTE_O2 0x55

boolean valid_stored_settings()
{
	return (settings.checkByte1 == CHECK_BYTE_O1 && settings.checkByte2 == CHECK_BYTE_O2);
}

void reset_settings()
{
	Serial.println("Settings reset");
	settings.version = 1;
	settings.checkByte1 = CHECK_BYTE_O1;
	settings.checkByte2 = CHECK_BYTE_O2;

	strcpy(settings.deviceNane, "sensor01");

	settings.wiFiOn = true;

	// enter some pre-set WiFi locations
	strcpy(settings.wifiSettings[0].wifiSsid, "sdfsdf");
	strcpy(settings.wifiSettings[0].wifiPassword, "sdfsfd");

	strcpy(settings.wifiSettings[1].wifiSsid, "sdfdfdf");
	strcpy(settings.wifiSettings[1].wifiPassword, "fdfsddf");

	strcpy(settings.wifiSettings[2].wifiSsid, "dsdsdffd");
	strcpy(settings.wifiSettings[2].wifiPassword, "fdsffdsf");

	// clear the rest of the settings
	for (int i = 3; i < NO_OF_WIFI_SETTINGS; i++)
	{
		settings.wifiSettings[i].wifiSsid[0] = 0;
		settings.wifiSettings[i].wifiPassword[0] = 0;
	}

	settings.seconds_per_mqtt_update = SECONDS_PER_MQTT_UPDATE;
	settings.seconds_per_mqtt_retry = SECONDS_PER_MQTT_RETRY;

	// You can put default settings here
	// These are copied into the device when it is first started

	settings.airqSensorType = ZPH01_SENSOR; 
	settings.airqLowLimit = 1;
	settings.airqLowWarnLimit = 2;
	settings.airqMidWarnLimit = 3;
	settings.airqHighWarnLimit = 4;
	settings.airQHighAlertLimit = 5;

	settings.noOfPixels = 12;
	settings.pixel_red = 0;
	settings.pixel_blue = 0;
	settings.pixel_green = 1.0;

#ifdef SECURE_SOCKETS

	strcpy(settings.mqttServer, "mydevice.azure-devices.net");
	settings.mqttPort = 8883;
	strcpy(settings.mqttName, "MQTTMini01");
	strcpy(settings.mqttUser, "mydevice.azure-devices.net/MQTTMini01");
	strcpy(settings.mqttPassword, "SharedAccessSignature sr=mydevice.azure-devices.net%2Fdevices%2FMQTTMini01&sig=sdfsddsdsffsdfsdfssfSFcRG%2BaeE%3D&se=1568130534");

	strcpy(settings.mqttPublishTopic, "devices/MQTTMini01/messages/events/");
	strcpy(settings.mqttSubscribeTopic, "devices/MQTTMini01/messages/devicebound/#");

#else

	strcpy(settings.mqttServer, "mqtt.connectedhumber.org");
	settings.mqttPort = 1883;
	strcpy(settings.mqttUser, "sdfsdf");
	strcpy(settings.mqttPassword, "sdfsdsdf");
	strcpy(settings.mqttName, "Sensor01");
	strcpy(settings.mqttPublishTopic, "sensor01/data");
	strcpy(settings.mqttSubscribeTopic, "sensor01/commands");


	strcpy(settings.mqttServer, "mqtt.connectedhumber.org");
	settings.mqttPort = 1883;
	strcpy(settings.mqttUser, "connectedhumber");
	strcpy(settings.mqttPassword, "3fds8gssf6");
	strcpy(settings.mqttName, "Sensor02");
	strcpy(settings.mqttPublishTopic, "sensor02/data");
	strcpy(settings.mqttSubscribeTopic, "sensor02/commands");


#endif
}

void serial_deliver_command_result(char * result)
{
	Serial.println(result);
}

#define SERIAL_BUFFER_SIZE 240
#define SERIAL_BUFFER_LIMIT SERIAL_BUFFER_SIZE-1

char serial_receive_buffer[SERIAL_BUFFER_SIZE];

int serial_receive_buffer_pos = 0;

void reset_serial_buffer()
{
	serial_receive_buffer_pos = 0;
}

void act_on_serial_command()
{
	act_onJson_command(serial_receive_buffer, serial_deliver_command_result);
}

void buffer_char(char ch)
{
	if ((serial_receive_buffer_pos > 0) &&
		(ch == '\n' || ch == '\r' || ch == 0))
	{
		// terminate the received string
		serial_receive_buffer[serial_receive_buffer_pos] = 0;
		act_on_serial_command();
		reset_serial_buffer();
	}
	else
	{
		if (serial_receive_buffer_pos < SERIAL_BUFFER_SIZE)
		{
			serial_receive_buffer[serial_receive_buffer_pos] = ch;
			serial_receive_buffer_pos++;
		}
	}
}


#define CHAR_FLUSH_START_TIMEOUT 1000

unsigned long character_timer_start;

void check_serial_buffer()
{
	if (Serial.available())
	{
		character_timer_start = millis();
		while (Serial.available())
		{
			buffer_char(Serial.read());
		}
	}
	else
	{
		if (serial_receive_buffer_pos > 0)
		{
			// have got some characters - if they've been here a while - discard them
			unsigned long elapsed_time = millis() - character_timer_start;
			if (elapsed_time > CHAR_FLUSH_START_TIMEOUT)
			{
				reset_serial_buffer();
			}
		}
	}
}

void setup_commands()
{
	EEPROM.begin(EEPROM_SIZE);

	load_settings();

	if (!valid_stored_settings())
	{
		TRACELN("Stored settings reset");
		reset_settings();
		save_settings();
	}
	else
	{
		TRACELN("Settings loaded OK");
	}
}

// Checks the serial port for any commands and acts on them

void loop_commands()
{
	check_serial_buffer();
}
