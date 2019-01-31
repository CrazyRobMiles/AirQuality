#pragma once

// from menu.h
void start_action(String title, String text);
void update_action(String title, String text);
void end_action();

// timeout in millis
#define WIFI_START_TIMEOUT 40000
#define WIFI_DISPLAY_TIMEOUT 3000
#define WIFI_STATION_SCAN_TIMEOUT 2000

unsigned long wifi_timer_start;

#define MIN_STR_LEN 8
#define MAX_STR_LEN 17

#define SSID_DEFAULT "MQTTMini"

std::unique_ptr<DNSServer> dnsServer;
const byte DNS_PORT = 53;

const char configPageHeader[] =
"<html>"
"<head>"
//"<style>input {font-size: 1.2em; width: 100%; max-width: 360px; display: block; margin: 5px auto; } </style>"
"<style>input {margin: 5px auto; } </style>"
"</head>"
"<body>"
"<h1> Monitair Configuration</h1>"
"<form id='form' action='/' method='post'>";

const char adminPageFooter[] =
"<input type='submit' value='Update'>"
"</form>"
"</body>"
"</html>";

#define WEB_PAGE_BUFFER_SIZE 5000

char webPageBuffer[WEB_PAGE_BUFFER_SIZE];

#define ERROR_MESSAGE_BUFFER_SIZE 2000
char errorMessageBuffer[ERROR_MESSAGE_BUFFER_SIZE];

void buildWebPageBuffer(char * webPgeBuffer, int bufferSize)
{
	int * valuePointer;
	boolean * boolPointer;

	snprintf(webPageBuffer, bufferSize, "%s", configPageHeader);

	for (int i = 0; i < sizeof(settingItems) / sizeof(struct SettingItem); i++)
	{
		snprintf(webPageBuffer, bufferSize, "%s <label for='%s'> %s: </label>",
			webPageBuffer,
			settingItems[i].formName,
			settingItems[i].prompt);

		switch (settingItems[i].settingType)
		{
		case text:
			snprintf(webPageBuffer, bufferSize, "%s <input name = '%s' type = 'text' value='%s'><br>",
				webPageBuffer, settingItems[i].formName, settingItems[i].value);
			break;
		case password:
			snprintf(webPageBuffer, bufferSize, "%s <input name = '%s' type = 'password' value='%s'><br>",
				webPageBuffer, settingItems[i].formName, settingItems[i].value);
			break;
		case number:
			valuePointer = (int*)settingItems[i].value;
			snprintf(webPageBuffer, bufferSize, "%s <input name = '%s' type = 'text' value='%d'><br>",
				webPageBuffer, settingItems[i].formName, *valuePointer);
			break;
		case onOff:
			boolPointer = (boolean*)settingItems[i].value;
			if (*boolPointer)
			{
				snprintf(webPageBuffer, bufferSize, "%s <input name = '%s' type = 'text' value='on'><br>",
					webPageBuffer, settingItems[i].formName);
			}
			else
			{
				snprintf(webPageBuffer, bufferSize, "%s <input name = '%s' type = 'text' value='off'><br>",
					webPageBuffer, settingItems[i].formName);
			}
			break;
		case yesNo:
			boolPointer = (boolean*)settingItems[i].value;
			if (*boolPointer)
			{
				snprintf(webPageBuffer, bufferSize, "%s <input name = '%s' type = 'text' value='yes'><br>",
					webPageBuffer, settingItems[i].formName);
			}
			else
			{
				snprintf(webPageBuffer, bufferSize, "%s <input name = '%s' type = 'text' value='no'><br>",
					webPageBuffer, settingItems[i].formName);
			}
			break;
		}
	}

	snprintf(webPageBuffer, bufferSize, "%s %s",
		webPageBuffer, adminPageFooter);
}

const char replyPageHeader[] = "<html><head></head><body>";

const char replyPageFooter[] = "New settings will take effect after restart. Disconnect the MQTT device from power, turn it the right way up and power it up again.</body></html>";

void serveWifiConfig(ESP8266WebServer *webServer) 
{
	Serial.println("Wifi config hit");
}

void serveHardwareConfig(ESP8266WebServer *webServer)
{
	Serial.println("Hardware config hit");
}

void serveMQTTConfig(ESP8266WebServer *webServer)
{
	Serial.println("MQTT config hit");
}

const char replyPageHeader[] = "<html><head></head><body>";

const char replyPageFooter[] = "New settings will take effect after restart. Disconnect the MQTT device from power, turn it the right way up and power it up again.</body></html>";

void serveHome(ESP8266WebServer *webServer) 
{

	TRACELN("Serve request hit");

	// Empty the error message string
	snprintf(errorMessageBuffer, ERROR_MESSAGE_BUFFER_SIZE, "%s", replyPageHeader);

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

			switch (settingItems[i].settingType)
			{
			case text:
				strcpy((char *)settingItems[i].value, argValue.c_str());
				break;
			case password:
				strcpy((char *)settingItems[i].value, argValue.c_str());
				break;
			case number:
				int value;
				if (sscanf(argValue.c_str(), "%d", &value) == 1)
				{
					*(int *)settingItems[i].value = value;
				}
				else
				{
					snprintf(errorMessageBuffer, ERROR_MESSAGE_BUFFER_SIZE, "%s <p>Invalid value %s for %s</p> ",
						errorMessageBuffer, argValue.c_str(), settingItems[i].prompt);
				}
				break;
			case onOff:
				if (strcasecmp(argValue.c_str(), "on") == 0)
				{
					*(boolean *)settingItems[i].value = true;
				}
				else
				{
					if (strcasecmp(argValue.c_str(), "off") == 0)
					{
						*(boolean *)settingItems[i].value = false;
					}
					else
					{
						snprintf(errorMessageBuffer, ERROR_MESSAGE_BUFFER_SIZE, "%s <p>Invalid value %s for %s</p> ",
							errorMessageBuffer, argValue.c_str(), settingItems[i].prompt);
					}
				}
				break;
			case yesNo:
				if (strcasecmp(argValue.c_str(), "yes") == 0)
				{
					*(boolean *)settingItems[i].value = true;
				}
				else
				{
					if (strcasecmp(argValue.c_str(), "no") == 0)
					{
						*(boolean *)settingItems[i].value = true;
					}
					else
					{
						snprintf(errorMessageBuffer, ERROR_MESSAGE_BUFFER_SIZE, "%s <p>Invalid value %s for %s</p> ",
							errorMessageBuffer, argValue.c_str(), settingItems[i].prompt);
					}
				}
				break;
			}
			TRACE(fName);
			TRACE(settingItems[i].value);
			TRACELN(argValue);
		}

		dump_settings();

		save_settings();

		snprintf(errorMessageBuffer, ERROR_MESSAGE_BUFFER_SIZE, "%s %s", errorMessageBuffer, replyPageFooter);

		webServer->sendHeader("Content-Length", String(strlen(errorMessageBuffer)));
		webServer->send(200, "text/html", errorMessageBuffer);

		delay(500);

		wifiState = WiFiSetupProcessingResponse;
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
	Serial.println("Starting wifi config");
	TRACELN("Starting WiFi config");
	// If the EEROM isn't valid then create a unique name for the wifi

	sprintf(configSSID, "%s %06X", SSID_DEFAULT, ESP.getChipId());

	WiFi.mode(WIFI_AP);
	delay(100);

	WiFi.softAP(configSSID);

	dnsServer.reset(new DNSServer());

	dnsServer->setErrorReplyCode(DNSReplyCode::NoError);

	delay(500);

	dnsServer->start(DNS_PORT, "*", WiFi.softAPIP());

	webServer = new ESP8266WebServer(80);

	buildWebPageBuffer(webPageBuffer, WEB_PAGE_BUFFER_SIZE);

	// Set up the admin page
	webServer->on("/", std::bind(serveHome, webServer));
	webServer->begin();
}

void startWifi()
{
	if (settings.wiFiOn)
	{
		wifi_timer_start = millis();
		wifiState = WiFiStarting;
	}
	else
	{
		wifiState = WiFiOff;
	}
}

void startWifiSetup()
{
	wifiState = WiFiSetupStarting;
}

void setup_wifi()
{
	wifiState = WiFiOff;
}

#define WIFI_SETTING_NOT_FOUND -1

int find_wifi_setting(String ssidName)
{
	char ssidBuffer[WIFI_SSID_LENGTH];

	ssidName.toCharArray(ssidBuffer, WIFI_SSID_LENGTH);

	TRACE("Checking: ");
	TRACELN(ssidName);

	for (int i = 0; i < NO_OF_WIFI_SETTINGS; i++)
	{
		if (strcasecmp(settings.wifiSettings[i].wifiSsid, ssidBuffer) == 0)
		{
			return i;
		}
	}
	return WIFI_SETTING_NOT_FOUND;
}

void loop_wifi()
{
	unsigned long elapsed_time;
	int no_of_networks;
	int setting_number;

	switch (wifiState)
	{

	case WiFiSetupOff:
		break;

	case WiFiSetupStarting:
		startWifiConfig();
		wifiState = WiFiSetupAwaitingClients;
		break;

	case WiFiSetupAwaitingClients:
		if (WiFi.softAPgetStationNum() > 0)
		{
			wifiState = WiFiSetupServingPage;
		}
		break;

	case WiFiSetupServingPage:
		webServer->handleClient();
		dnsServer->processNextRequest();
		break;

	case WiFiSetupProcessingResponse:
		wifiState = WiFiSetupDone;
		break;

	case WiFiSetupDone:
		break;

	case WiFiStarting:
		TRACELN("WiFi Connection starting");
		WiFi.scanNetworks(true); // perform an asynchronous scan
		start_action("WiFi", "Scanning");
		wifiState = WiFiScanning;
		break;

	case WiFiScanning:

		no_of_networks = WiFi.scanComplete();

		if (no_of_networks == WIFI_SCAN_RUNNING)
		{
			break;
		}

		if (no_of_networks != WIFI_SCAN_FAILED)
		{
			// if we get here we have some networks
			for (int i = 0; i < no_of_networks; ++i) {
				setting_number = find_wifi_setting(WiFi.SSID(i));
				if (setting_number != WIFI_SETTING_NOT_FOUND)
				{
					TRACE("Found: ");
					TRACELN(settings.wifiSettings[setting_number].wifiSsid);
					TRACE("PWD: ");
					TRACELN(settings.wifiSettings[setting_number].wifiPassword);
					WiFi.begin(settings.wifiSettings[setting_number].wifiSsid,
						settings.wifiSettings[setting_number].wifiPassword);
					update_action(settings.wifiSettings[setting_number].wifiSsid,
						"Connecting");
					wifiState = WiFiConnecting;
					break;
				}
			}
		}

		if (wifiState != WiFiConnecting)
		{
			// didn't find a matching network
			update_action("WiFi", "No networks");
			wifi_timer_start = millis();
			wifiState = WiFiWaitingForNextScan;
		}

		break;

	case WiFiWaitingForNextScan:

		elapsed_time = millis() - wifi_timer_start;

		if (elapsed_time > WIFI_STATION_SCAN_TIMEOUT)
		{
			WiFi.scanNetworks(true); // perform an asynchronous scan
			start_action("WiFi", "Scanning");
			wifiState = WiFiScanning;
		}

		break;

	case WiFiConnecting:

		if (WiFi.status() == WL_CONNECTED)
		{
			update_action("WiFi", "Connected OK");
			wifiState = ShowingWifiConnected;
			wifi_timer_start = millis();
		}

		elapsed_time = millis() - wifi_timer_start;

		if (elapsed_time > WIFI_START_TIMEOUT)
		{
			wifi_timer_start = millis();
			update_action("WiFi", "Connect failed");
			wifiState = WiFiConnectFailed;
		}

		break;

	case ShowingWifiConnected:

		elapsed_time = millis() - wifi_timer_start;

		if (elapsed_time > WIFI_DISPLAY_TIMEOUT)
		{
			end_action();
			wifiState = WiFiConnected;
		}
		break;

	case WiFiConnectFailed:
		elapsed_time = millis() - wifi_timer_start;
		if (elapsed_time > WIFI_DISPLAY_TIMEOUT)
		{
			wifiState = WiFiNotConnected;
			end_action();
		}
		break;

	case WiFiConnected:
		break;

	case WiFiNotConnected:
		break;

	case WiFiOff:
		break;
	}
}

