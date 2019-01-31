#pragma once

#define STARTUP_STATE_TIME 20000

unsigned long millisAtStartup;

void setShowStatusState()
{
	deviceState = showStatus;
}

void setActiveState()
{
	deviceState = active;
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

	if (inputState == pressed)
	{
		startWifiSetup();
		deviceState = wifiSetup;
	}
	else
	{
		startWifi();
		deviceState = starting;
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