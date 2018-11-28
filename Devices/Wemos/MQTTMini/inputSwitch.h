#pragma once

// tilt switch is across D2 and D1 on the Wemos D1 Mini
// Note that using GPIO4 and GPIO0 D2 and D3 does not work. And it prevents program upload. 
// Something special about GPIO0 it seems.
// Using 4 and 5 works fine = 

#define GROUND_PIN 5
#define INPUT_PIN 4

enum InputState { upright, inverted };

InputState inputState;

int lastInputValue;
int inputDebounceCount;

unsigned long millisAtLastInputChange;

#define NUMBER_OF_INPUT_CLIENTS 5

struct inputHandler {
	void(*boxInvertedHandler)(long millisSinceChange);
	void(*boxUprightHandler)(long millisSinceChange);
	bool assigned;
};

struct inputHandler inputHandlers[NUMBER_OF_INPUT_CLIENTS];

void resetInputHandlers()
{
	for (int i = 0; i < NUMBER_OF_INPUT_CLIENTS; i++)
	{
		inputHandlers[i].assigned = false;
	}
}

// bind an input handler to the box being inverted - returns true if the binding worked
// returns the number of the handler added or -1 if the add failed
int addInputHandler(void(*boxInvertedHandler)(long millisSinceChange),
	void(*boxUprightHandler)(long millisSinceChange))
{
	for (int i = 0; i < NUMBER_OF_INPUT_CLIENTS; i++)
	{
		if (inputHandlers[i].assigned == false)
		{
			inputHandlers[i].assigned = true;
			inputHandlers[i].boxInvertedHandler = boxInvertedHandler;
			inputHandlers[i].boxUprightHandler = boxUprightHandler;
			return i;
		}
	}
	return -1;
}

// remove the given handler from the input 

void removeInputHandler(int handlerNo)
{
	if (handlerNo >= 0 && handlerNo < NUMBER_OF_INPUT_CLIENTS)
	{
		inputHandlers[handlerNo].assigned = false;
	}
}

void sendBoxInverted(long millisSinceChange)
{
	for (int i = 0; i < NUMBER_OF_INPUT_CLIENTS; i++)
	{
		if (inputHandlers[i].assigned)
		{
			inputHandlers[i].boxInvertedHandler(millisSinceChange);
		}
	}
}

void sendBoxUpright(long millisSinceChange)
{
	for (int i = 0; i < NUMBER_OF_INPUT_CLIENTS; i++)
	{
		if (inputHandlers[i].assigned)
		{
			inputHandlers[i].boxUprightHandler(millisSinceChange);
		}
	}
}

#define INPUT_DEBOUNCE_LIMIT 200

void setInputValue(int value)
{
	lastInputValue = value;
	inputDebounceCount = 0;

	long currentMillis = millis();
	long millisSinceChange = ulongDiff(currentMillis, millisAtLastInputChange);

	if (lastInputValue)
	{
		inputState = inverted;
		sendBoxInverted(millisSinceChange);
	}
	else
	{
		inputState = upright;
		sendBoxUpright(millisSinceChange);
	}

	millisAtLastInputChange = currentMillis;
}

void doTestInverted(long timeChange)
{
	Serial.print("Inverted: ");
	Serial.println(timeChange);
}

void doTestUpright(long timeChange)
{
	Serial.print("Upright: ");
	Serial.println(timeChange);
}

void bindTests()
{
	addInputHandler(doTestInverted, doTestUpright);
}


void setup_input()
{
	pinMode(GROUND_PIN, OUTPUT);
	digitalWrite(GROUND_PIN, 0);
	pinMode(INPUT_PIN, INPUT_PULLUP);
	resetInputHandlers();
	setInputValue(digitalRead(INPUT_PIN));

//	bindTests();
}

void loop_input()
{
	int newInputValue = digitalRead(INPUT_PIN);

	if (newInputValue == lastInputValue)
	{
		inputDebounceCount = 0;
	}
	else
	{
		if (++inputDebounceCount > INPUT_DEBOUNCE_LIMIT)
		{
			setInputValue(newInputValue);
		}
	}
}