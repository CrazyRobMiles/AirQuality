#include <Arduino.h>

#include "utils.h"
#include "inputswitch.h"
#include "settings.h"

int lastInputValue;
long inputDebounceStartTime;

boolean switchValue;

boolean getInputSwitchValue()
{
	return switchValue;
}

unsigned long millisAtLastInputChange;

// number of milliseconds that the signal must be stable
// before we read it

#define INPUT_DEBOUNCE_TIME 10


int startInputSwitch(struct process * inputSwitchProcess)
{
	pinMode(settings.controlInputPin, INPUT_PULLUP);
	return PROCESS_OK;
}

int updateInputSwitch(struct process * inputSwitchProcess)
{
	int newInputValue = digitalRead(settings.controlInputPin);

	if (newInputValue == lastInputValue)
	{
		inputDebounceStartTime = millis();
	}
	else
	{
		long currentMillis = millis();
		long millisSinceChange = ulongDiff(currentMillis, inputDebounceStartTime);

		if (++millisSinceChange > INPUT_DEBOUNCE_TIME)
		{
			if (newInputValue)
				switchValue = !settings.controlInputPinActiveLow;
			else
				switchValue = settings.controlInputPinActiveLow;
			lastInputValue = newInputValue;
		}
	}
	return PROCESS_OK;
}

int stopInputSwitch(struct process * inputSwitchProcess)
{
	return INPUT_SWITCH_STOPPED;
}

void inputSwitchStatusMessage(struct process * inputSwitchProcess, char * buffer, int bufferLength)
{
	if (switchValue)
		snprintf(buffer, bufferLength, "Input switch pressed");
	else
		snprintf(buffer, bufferLength, "Input switch released");
}


boolean readInputSwitch()
{
	int newInputValue;
	int lastInputValue = digitalRead(settings.controlInputPin);
	long inputDebounceStartTime = millis();

	while (true)
	{
		int newInputValue = digitalRead(settings.controlInputPin);

		if (newInputValue != lastInputValue)
		{
			inputDebounceStartTime = millis();
			lastInputValue = newInputValue;
		}
		else
		{
			long currentMillis = millis();
			long millisSinceChange = ulongDiff(currentMillis, inputDebounceStartTime);

			if (++millisSinceChange > INPUT_DEBOUNCE_TIME)
			{
				if (newInputValue)
					return !settings.controlInputPinActiveLow;
				else
					return settings.controlInputPinActiveLow;
			}
		}
	}
}
