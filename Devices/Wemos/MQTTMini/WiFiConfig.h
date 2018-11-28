#pragma once

#define MIN_STR_LEN 8
#define MAX_STR_LEN 17

#define SSID_DEFAULT "APConfig"

std::unique_ptr<DNSServer> dnsServer;
const byte DNS_PORT = 53;

struct SettingItem {
	char * prompt;
	char * formName;
	char * value;
	bool isPassword;
};

struct SettingItem settingItems[] =
{
	"WiFiSSID1: ", "wifissid1", settings.wifiSettings[0].wifiSsid, false,
	"WiFiPassword1: ", "wifipwd1", settings.wifiSettings[0].wifiPassword, true,
	"WiFiSSID2: ", "wifissid2", settings.wifiSettings[1].wifiSsid, false,
	"WiFiPassword2: ", "wifipwd2", settings.wifiSettings[1].wifiPassword, true, 
	"WiFiSSID3: ", "wifissid3", settings.wifiSettings[2].wifiSsid, false, 
	"WiFiPassword3: ", "wifipwd3", settings.wifiSettings[2].wifiPassword, true, 
	"WiFiSSID4: ", "wifissid4", settings.wifiSettings[3].wifiSsid, false, 
	"WiFiPassword4: ", "wifipwd4", settings.wifiSettings[3].wifiPassword, true, 
	"WiFiSSID5: ", "wifissid5", settings.wifiSettings[4].wifiSsid, false,
	"WiFiPassword5: ", "wifipwd5", settings.wifiSettings[4].wifiPassword, true, 
	"MQTT DeviceName: ", "mname", settings.mqttName,false, 
	"MQTT Host: ", "mhost", settings.mqttServer, false,
	"MQTT UserName: ", "muser", settings.mqttUser,false,
	"MQTT Password: ", "mpwd", settings.mqttPassword, true,
	"MQTT Publish topic: ", "mpub", settings.mqttPublishTopic, false,
	"MQTT Subscribe topic: ", "msub", settings.mqttSubscribeTopic, false
};


const char adminPageHeader[] =
"<html>"
"<head>"
//"<style>input {font-size: 1.2em; width: 100%; max-width: 360px; display: block; margin: 5px auto; } </style>"
"<style>input {margin: 5px auto; } </style>"
"</head>"
"<body>"
"<h1> MQTTT Mini Configuration</h1>"
"<form id='form' action='/' method='post'>";

const char adminPageFooter[]=
"<input type='submit' value='Update'>"
"</form>"
"</body>"
"</html>";

#define WEB_PAGE_BUFFER_SIZE 5000

char webPageBuffer[WEB_PAGE_BUFFER_SIZE];

#define TOPIC_BUFFER_SIZE 200

char topicBuffer[TOPIC_BUFFER_SIZE];

void buildWebPageBuffer()
{
	strcpy(webPageBuffer, adminPageHeader);
	
	for (int i = 0; i < sizeof(settingItems) / sizeof(struct SettingItem); i++)
	{
		sprintf(topicBuffer, "<label for='%s'> %s </label>",
			settingItems[i].formName,
			settingItems[i].prompt);

		strcat(webPageBuffer, topicBuffer);

		if (settingItems[i].isPassword)
		{
			sprintf(topicBuffer, "<input name = '%s' type = 'password' value='%s'><br>",
				settingItems[i].formName, settingItems[i].value);
		}
		else
		{
			sprintf(topicBuffer, "<input name = '%s' type = 'text' value='%s'><br>",
				settingItems[i].formName, settingItems[i].value);
		}
		strcat(webPageBuffer, topicBuffer);
	}

	strcat(webPageBuffer, adminPageFooter);
}

void serveAdmin(ESP8266WebServer *webServer) {

	TRACELN("Serve request hit");

	String message;

	// Check to see if we've been sent any arguments and instantly return if not
	if (webServer->args() == 0) {
		TRACELN("Serving the page");
		webServer->sendHeader("Content-Length", String(strlen(webPageBuffer)));
		webServer->send(200, "text/html", webPageBuffer);
	}
	else {
		// put the results back into the settings
		for (int i = 0; i < sizeof(settingItems) / sizeof(struct SettingItem); i++)
		{
			String fName = String(settingItems[i].formName);
			String argValue = webServer->arg(fName);
			strcpy(settingItems[i].value,argValue.c_str());
			TRACE(fName);
			TRACE(settingItems[i].value);
			TRACELN(argValue);
		}

		dump_settings();

		save_settings();

		// Construct a message to tell the user that the change worked
		message = "New settings will take effect after restart. Disconnect the MQTT device from power, turn it the right way up and power it up again.";

		// Reply with a web page to indicate success or failure
		message = "<html><head></head><body>" + message;
		message += "</body></html>";
		webServer->sendHeader("Content-Length", String(message.length()));
		webServer->send(200, "text/html", message);

		wifiSetupState = WiFiSetupProcessingResponse;
	}
}


// Checks all of the bytes in the string array to make sure they are valid characters
bool ValidateString(char* value) {
	bool valid = true;

	//Check to see if the string is an acceptable length
	if ((strlen(value) < MIN_STR_LEN) && (strlen(value) >= MAX_STR_LEN)) {
		valid = false;
	}
	else {
		//Check each character in the string to make sure it is alphanumeric or space
		for (uint8_t i = 0; i < strlen(value); i++)
			if (!isAlphaNumeric(value[i]))
				if (!isSpace(value[i]))
					valid = false;
	}

	return valid;
}

char configSSID[MAX_STR_LEN];

ESP8266WebServer * webServer;

void startWifiConfig()
{
	TRACELN("Starting WiFi config");
	// If the EEROM isn't valid then create a unique name for the wifi
	
	sprintf(configSSID, "%s %06X", SSID_DEFAULT, ESP.getChipId());

	WiFi.softAP(configSSID);

	dnsServer.reset(new DNSServer());

	dnsServer->setErrorReplyCode(DNSReplyCode::NoError);

	delay(500);

	dnsServer->start(DNS_PORT, "*", WiFi.softAPIP());

	webServer = new ESP8266WebServer(80);

	buildWebPageBuffer();

	// Set up the admin page
	webServer->on("/", std::bind(serveAdmin, webServer));
	webServer->begin();

	wifiSetupState = WiFiSetupAwaitingClients;
}

void setup_wifiConfig()
{
}


void loop_wifiConfig()
{
	switch (wifiSetupState)
	{
	case WiFiSetupOff:
		break;

	case WiFiSetupAwaitingClients:
		if (WiFi.softAPgetStationNum() > 0)
		{
			wifiSetupState = WiFiSetupServingPage;
		}
		break;

	case WiFiSetupServingPage:
	case WiFiSetupProcessingResponse:
		webServer->handleClient();
		dnsServer->processNextRequest();
		break;

		break;
	}
}

