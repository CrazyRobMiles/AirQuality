#pragma once

enum InputState { upright, inverted };

InputState inputState;

int lastInputValue;
long inputDebounceStartTime;

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

// bind an input handler to the box being inverted 
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

// number of milliseconds that the signal must be stable
// before we read it

#define INPUT_DEBOUNCE_TIME 100



void setInputValue(int value)
{
	lastInputValue = value;
	inputDebounceStartTime = millis();

	ulong currentMillis = millis();
	ulong millisSinceChange = ulongDiff(currentMillis, millisAtLastInputChange);

	if (!lastInputValue)
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

// Reads the value and blocks until it gets a valid one
void readInputValue()
{
	inputDebounceStartTime = millis();

	int lastInputValue = digitalRead(CONFIG_INPUT_PIN);

	while (true)
	{
		int newInputValue = digitalRead(CONFIG_INPUT_PIN);

		if (newInputValue == lastInputValue)
		{
			long currentMillis = millis();
			long millisSinceChange = ulongDiff(currentMillis, inputDebounceStartTime);

			if (++millisSinceChange > INPUT_DEBOUNCE_TIME)
			{
				setInputValue(newInputValue);
				break;
			}
		}
		else
		{
			inputDebounceStartTime = millis();
		}
		delay(1);
	}
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
	pinMode(CONFIG_INPUT_PIN, INPUT_PULLUP);
	resetInputHandlers();
	readInputValue();

//	bindTests();
}

void loop_input()
{
	int newInputValue = digitalRead(CONFIG_INPUT_PIN);

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
			setInputValue(newInputValue);
		}
	}
}