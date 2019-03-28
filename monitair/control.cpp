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

unsigned long millisAtLastSend;
bool initialPowerUp = true;

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
	unsigned long currentMillis = millis();

	if (initialPowerUp ||
		ulongDiff(currentMillis, millisAtLastSend) > (settings.mqttSecsPerUpdate * 1000))
	{
		if (1) //activeMQTTProcess->status == MQTT_OK)
		{
			initialPowerUp = false;
			millisAtLastSend = currentMillis;
			createSensorJson(jsonBuffer, JSON_BUFFER_SIZE);

			if (publishReadingsToMQTT(jsonBuffer))
			{
				//Serial.println(jsonBuffer);
			}
		}
	}
}