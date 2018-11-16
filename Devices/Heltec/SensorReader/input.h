
#define UP_BUTTON 12
#define DOWN_BUTTON 0
#define SELECT_BUTTON 23

void up_pressed()
{
	TRACELN("Up Pressed");
	menu_up_pressed();
}

void down_pressed()
{
	TRACELN("Down Pressed");
	menu_down_pressed();
}

void select_pressed()
{
	TRACELN("Select Pressed");
	menu_select_pressed();
}

void up_released()
{
	TRACELN("Up released");
}

void down_released()
{
	TRACELN("Down released");
}

void select_released()
{
	TRACELN("Select released");
}


struct Button {
	void(*pressed_handler)();
	void(*released_handler)();
	int pinNo;
	bool lastState;
};

struct Button buttons[] = { {up_pressed, up_released, UP_BUTTON, false }, { down_pressed, down_released, DOWN_BUTTON, false }, {select_pressed, select_released, SELECT_BUTTON, false } };

int noOfButtons = sizeof(buttons) / sizeof(struct Button);

void setup_input() 
{
	for (int i = 0; i < noOfButtons; i++)
	{
		pinMode(buttons[i].pinNo, INPUT_PULLUP);
		buttons[i].lastState = digitalRead(buttons[i].pinNo);
	}
}

void loop_input()
{
	for (int i = 0; i < noOfButtons; i++)
	{
		bool state = digitalRead(buttons[i].pinNo);
		if (state != buttons[i].lastState)
		{
			// change of state
			if (state)
			{
				buttons[i].released_handler();
			}
			else
			{
				buttons[i].pressed_handler();
			}
		}
		buttons[i].lastState = state;
	}
}

