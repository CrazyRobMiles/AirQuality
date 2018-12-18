#pragma once

#define STARTUP_STATE_TIME 20000

unsigned long millisAtStartup;

void setShowStatusState()
{
	deviceState = showStatus;
	resetPixelStatus();
}

void setActiveState()
{
	deviceState = active;
	setupWalkingColour({ 0,1,0 });
}

void doManageInverted(long timeChange)
{
	setShowStatusState();
}

void doManageUpright(long timeChange)
{
	setActiveState();
}

void setup_StateManager()
{
	readInputValue();

	if (inputState == inverted)
	{
		startWifiSetup();
		deviceState = wifiSetup;
		resetPixelStatus();
	}
	else
	{
		startWifi();
		deviceState = starting;
		resetPixelStatus();
	}

	millisAtStartup = millis();
	addInputHandler(doManageInverted, doManageUpright);
}

void loop_StateManager()
{
	unsigned long currentMillis = millis();
	unsigned long millisSinceStartup = ulongDiff(currentMillis, millisAtStartup);

	switch (deviceState)
	{
	case wifiSetup:
		break;

	case showStatus:
		break;

	case starting:

		if (mqttState == ConnectedToMQTTServer)
		{
			setActiveState();
		}

		if(millisSinceStartup > STARTUP_STATE_TIME)
		{
			setActiveState();
		}

		break;

	case active:
		break;
	}
}