#pragma once
#define WEB_PAGE_BUFFER_SIZE 5000

#define WEBSERVER_OK 0
#define WEBSERVER_OFF 1
#define WEBSERVER_ERROR_NO_WIFI -1

char webPageBuffer[WEB_PAGE_BUFFER_SIZE];


const char homePageHeader[] =
"<html>"
"<head>"
//"<style>input {font-size: 1.2em; width: 100%; max-width: 360px; display: block; margin: 5px auto; } </style>"
"<style>input {margin: 5px auto; } </style>"
"</head>"
"<body>"
"<h1> Monitair Home</h1>"
"<h2>Version %d.%d</h2>"; // version number goes here;

const char homePageFooter[] =
"<p> Select the link to the page that you want to view.</p>"
"</body>"
"</html>";

void buildHomePage(char * webPageBuffer, int bufferSize)
{
	snprintf(webPageBuffer, bufferSize, homePageHeader, settings.majorVersion, settings.minorVersion);

	for (int collectionNo = 0; collectionNo < sizeof(allSettings) / sizeof(SettingItemCollection); collectionNo++)
	{
		snprintf(webPageBuffer, bufferSize, "%s <p><a href=""%s"">%s</a> </p>", 
			webPageBuffer,
			allSettings[collectionNo].collectionName,
			allSettings[collectionNo].collectionDescription);
	}

	snprintf(webPageBuffer, bufferSize, "%s %s",
		webPageBuffer, homePageFooter);
}

const char settingsPageHeader[] =
"<html>"
"<head>"
"<style>input {margin: 5px auto; } </style>"
"</head>"
"<body>"
"<h1> Monitair Configuration</h1>"
"<h2>%s</h2>" // configuration description goes here
"<form id='form' action='/%s' method='post'>"; // configuration short name goes here

const char settingsPageFooter[] =
"%s"// entire page goes here
"<input type='submit' value='Update'>" 
"</form>"
"</body>"
"</html>";

void buildCollectionSettingsPage(SettingItemCollection * settingCollection,  char * webPageBuffer, int bufferSize)
{
	int * valuePointer;
	double * doublePointer;
	boolean * boolPointer;

	snprintf(webPageBuffer, bufferSize, settingsPageHeader, settingCollection->collectionDescription, settingCollection->collectionName);

	// Start the search at setting collection 0 so that the quick settings are used to build the web page
	for (int i = 0; i < settingCollection->noOfSettings; i++)
	{
		snprintf(webPageBuffer, bufferSize, "%s <label for='%s'> %s: </label>",
			webPageBuffer,
			settingCollection->settings[i].formName,
			settingCollection->settings[i].prompt);

		switch (settingCollection->settings[i].settingType)
		{
		case text:
			snprintf(webPageBuffer, bufferSize, "%s <input name = '%s' type = 'text' value='%s'><br>",
				webPageBuffer, settingCollection->settings[i].formName, settingCollection->settings[i].value);
			break;
		case password:
			snprintf(webPageBuffer, bufferSize, "%s <input name = '%s' type = 'password' value='%s'><br>",
				webPageBuffer, settingCollection->settings[i].formName, settingCollection->settings[i].value);
			break;
		case integerValue:
			valuePointer = (int*)settingCollection->settings[i].value;
			snprintf(webPageBuffer, bufferSize, "%s <input name = '%s' type = 'text' value='%d'><br>",
				webPageBuffer, settingCollection->settings[i].formName, *valuePointer);
			break;
		case doubleValue:
			doublePointer = (double*)settingCollection->settings[i].value;
			snprintf(webPageBuffer, bufferSize, "%s <input name = '%s' type = 'text' value='%lf'><br>",
				webPageBuffer, settingCollection->settings[i].formName, *doublePointer);
			break;
		case onOff:
			boolPointer = (boolean*)settingCollection->settings[i].value;
			if (*boolPointer)
			{
				snprintf(webPageBuffer, bufferSize, "%s <input name = '%s' type = 'text' value='on'><br>",
					webPageBuffer, settingCollection->settings[i].formName);
			}
			else
			{
				snprintf(webPageBuffer, bufferSize, "%s <input name = '%s' type = 'text' value='off'><br>",
					webPageBuffer, settingCollection->settings[i].formName);
			}
			break;
		case yesNo:
			boolPointer = (boolean*)settingCollection->settings[i].value;
			if (*boolPointer)
			{
				snprintf(webPageBuffer, bufferSize, "%s <input name = '%s' type = 'text' value='yes'><br>",
					webPageBuffer, settingCollection->settings[i].formName);
			}
			else
			{
				snprintf(webPageBuffer, bufferSize, "%s <input name = '%s' type = 'text' value='no'><br>",
					webPageBuffer, settingCollection->settings[i].formName);
			}
			break;
		}
	}

	snprintf(webPageBuffer, bufferSize, settingsPageFooter,webPageBuffer);
}

const char replyPageHeader[] =
"<html>"
"<head>"
"<style>input {margin: 5px auto; } </style>"
"</head>"
"<body>"
"<h1> Monitair Configuration</h1>"
"<h2>%s</h2>"; // configuration description goes here

const char replyPageFooter[] = 
"%s" 
"<p>Settings updated.</p>"
"<p><a href = ""/"">return to the settings home screen </a></p>"
"</body></html>";

void updateSettings(ESP8266WebServer *webServer, SettingItemCollection * settingCollection, char * webPageBuffer, int bufferSize)
{
	snprintf(webPageBuffer, bufferSize, replyPageHeader, settingCollection->collectionDescription, settingCollection->collectionName);

	for (int i = 0; i < settingCollection->noOfSettings; i++)
	{
		String fName = String(settingCollection->settings[i].formName);
		String argValue = webServer->arg(fName);

		if (!settingCollection->settings[i].validateValue(
			settingCollection->settings[i].value,
			argValue.c_str()))
		{
			snprintf(webPageBuffer, bufferSize, "%s <p>Invalid value %s for %s</p> ",
				webPageBuffer, argValue.c_str(), settingCollection->settings[i].prompt);
		}
	}
	saveSettings();
	snprintf(webPageBuffer, bufferSize, replyPageFooter, webPageBuffer);
}

ESP8266WebServer * webServer = NULL;

void serveHome(ESP8266WebServer *webServer)
{
	// Serial.println("Serve request hit");

	if (webServer->args() == 0) {
		// Serial.println("Serving the home page");
		buildHomePage(webPageBuffer, WEB_PAGE_BUFFER_SIZE);
		webServer->sendHeader("Content-Length", String(strlen(webPageBuffer)));
		webServer->send(200, "text/html", webPageBuffer);
	}
}

const char pageNotFoundMessage[] =
"<html>"
"<head>"
"<style>input {margin: 5px auto; } </style>"
"</head>"
"<body>"
"<h1> Monitair Configuration</h1>"
"<h2>Page not found</h2>"
"<p>Sorry about that.</p>"
"</body></html>";

void pageNotFound(ESP8266WebServer *webServer)
{
	//Serial.println("Not found hit");

	String uriString = webServer->uri();

	const char * uriChars = uriString.c_str();
	const char * pageNameStart = uriChars + 1;

	SettingItemCollection * items = findSettingItemCollectionByName(pageNameStart);

	if (items != NULL)
	{
		// url refers to an existing set of settings
		if (webServer->args() == 0)
		{
			// Not a post - just serve out the settings form
			//Serial.printf("Got settings request for %s\n", items->collectionName);
			buildCollectionSettingsPage(items, webPageBuffer, WEB_PAGE_BUFFER_SIZE);
		}
		else
		{
			// Posting new values - now we need to read them
			//Serial.printf("Got new data for %s\n", items->collectionName);
			updateSettings(webServer, items, webPageBuffer, WEB_PAGE_BUFFER_SIZE);
		}
		webServer->sendHeader("Content-Length", String(strlen(webPageBuffer)));
		webServer->send(200, "text/html", webPageBuffer);
	}
	else
	{
		//Serial.printf("Page not found: %s\n", uriString.c_str());

		webServer->sendHeader("Content-Length", String(strlen(pageNotFoundMessage)));
		webServer->send(200, "text/html", pageNotFoundMessage);
	}
}

int startWebServer(struct process * webserverProcess)
{
	if (WiFiProcessDescriptor.status != WIFI_OK)
	{
		webserverProcess->status = WEBSERVER_ERROR_NO_WIFI;
		return WEBSERVER_ERROR_NO_WIFI;
	}

	webServer = new ESP8266WebServer(80);

	// Set up the admin page
	webServer->on("/", std::bind(serveHome, webServer));
	webServer->onNotFound(std::bind(pageNotFound, webServer));
	webServer->begin();
	webserverProcess->status = WEBSERVER_OK;
	return WEBSERVER_OK;
}

int updateWebServer(struct process * webserverProcess)
{
	if (webserverProcess->status == WEBSERVER_OK)
	{
		webServer->handleClient();
	}

	return WEBSERVER_OK;
}

int stopWebserver(struct process * webserverProcess)
{
	webserverProcess->status = WEBSERVER_OFF;
	return WEBSERVER_OFF;
}

void webserverStatusMessage(struct process * webserverProcess, char * buffer, int bufferLength)
{
	switch (webserverProcess->status)
	{
	case WEBSERVER_OK:
		snprintf(buffer, bufferLength, "Web server OK");
		break;
	case WEBSERVER_OFF:
		snprintf(buffer, bufferLength, "Web server OFF");
		break;
	case WEBSERVER_ERROR_NO_WIFI:
		snprintf(buffer, bufferLength, "No Wifi for the web server");
		break;
	default:
		snprintf(buffer, bufferLength, "Web server status invalid");
		break;
	}
}


void testWebServer()
{
	for (int collectionNo = 0; collectionNo < sizeof(allSettings) / sizeof(SettingItemCollection); collectionNo++)
	{
		buildCollectionSettingsPage(&allSettings[collectionNo], webPageBuffer, WEB_PAGE_BUFFER_SIZE);
		Serial.println(webPageBuffer);
		Serial.println();
	}

	buildHomePage(webPageBuffer, WEB_PAGE_BUFFER_SIZE);
	Serial.println(webPageBuffer);
}
