#pragma once


ulong powerInterval = 500;

ulong millisAtLastPowerChange;
byte powerState;

void setup_power()
{
	pinMode(POWER_CONTROL_PIN, OUTPUT);
	digitalWrite(POWER_CONTROL_PIN, 1);
	powerState = 1;
	millisAtLastPowerChange = millis();
}



void loop_power()
{
	long currentMillis = millis();
	long millisSinceChange = ulongDiff(currentMillis, millisAtLastPowerChange);
	if (millisSinceChange > powerInterval)
	{
		millisAtLastPowerChange = currentMillis;
		powerState = 1 - powerState;
		digitalWrite(POWER_CONTROL_PIN, powerState);
		TRACE("Power:");
		TRACELN(powerState);
	}
}