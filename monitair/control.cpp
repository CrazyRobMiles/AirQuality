#include "control.h"
#include "mqtt.h"
#include "pixels.h"
#include "inputswitch.h"
#include "sensors.h"
#include "settings.h"

void showDeviceStatus()
{
	beginStatusDisplay();
	displayProcessStatus();
	displaySensorStatus();
	renderStatusDisplay();
}

void startDevice()
{
	if (readInputSwitch())
	{
		Serial.println("Starting WiFi configuration");
		Serial.println("Reset the device to exit configuratin mode\n\n");
		beginWifiStatusDisplay();
		startWifiConfigProcesses();
		while (1)
		{
			updateWifiConfigProcesses();
		}
	}
	else
	{
		Serial.println("Starting node operation");
		beginStatusDisplay();
		startDeviceProcesses();
		startSensors();
		delay(1000); // show the status for a second
		setupWalkingColour(GREEN_PIXEL_COLOUR);
	}
}

#define JSON_BUFFER_SIZE 2000
char jsonBuffer[JSON_BUFFER_SIZE];

void sendSensorReadings()
{
	unsigned long cycleStartMillis = millis();

	updateSensors();

	createSensorJson(jsonBuffer, JSON_BUFFER_SIZE);

	if (publishReadingsToMQTT(jsonBuffer))
	{
		//Serial.println(jsonBuffer);
	}

	activeDelay(settings.mqttSecsPerUpdate * 1000 + 1);
}