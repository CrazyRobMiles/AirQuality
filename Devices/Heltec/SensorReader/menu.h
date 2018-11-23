
enum MenuState { workingStateActive, mainMenuStateActive, messageDisplayStateActive, readingNumberStateActive, performingActionActive };

MenuState menuState;

struct Menu
{
	uint8_t active_menu_item;
	String menuText;
	void(*methods[5])();
};


void set_working_state()
{
	TRACELN("Setting working state active");
	menuState = workingStateActive;
	set_working_display();
}

#define MENU_STACK_SIZE 15

Menu * menu_stack[MENU_STACK_SIZE];

// Stack top is the location of the top of the stack
// If it is -1 this means that the stack is empty
// Increment the stack top BEFORE adding anything to the stack

int8_t menu_stack_top = -1;

void push_menu(Menu * menu)
{
	TRACELN("Pushing menu");
	menu_stack_top++;
	menu_stack[menu_stack_top] = menu;
}

Menu * pop_menu()
{
	TRACELN("Popping menu");

	if (menu_stack_top == -1) {
		return NULL;
	}

	Menu * result = menu_stack[menu_stack_top];

	menu_stack_top--;
	return result;
}

Menu * peek_menu_stack_top()
{
	if (menu_stack_top == -1)
		return NULL;

	return menu_stack[menu_stack_top];
}

bool menu_stack_empty()
{
	return menu_stack_top == -1;
}

void enter_menu(Menu * menu)
{
	// record the position in the current menu for return
	if (!menu_stack_empty())
		peek_menu_stack_top()->active_menu_item = get_selected_item();

	push_menu(menu);

	set_menu_display(menu->menuText, menu->active_menu_item);

	menuState = mainMenuStateActive;
}

void exit_menu()
{
	// record the position in the current menu for return
	peek_menu_stack_top()->active_menu_item = get_selected_item();

	pop_menu();

	Menu * next_menu = peek_menu_stack_top();

	if (next_menu == NULL)
	{
		set_working_state();
		return;
	}

	set_menu_display(next_menu->menuText, next_menu->active_menu_item);
}

unsigned long delay_start_time;
unsigned long delay_in_millis;

void(*message_complete_callback)();

void display_message_screen(String title, String text, int in_delay_in_millis, void(*callback)())
{
	message_complete_callback = callback;
	set_message_display(title, text);
	menuState = messageDisplayStateActive;
	delay_in_millis = in_delay_in_millis;
	delay_start_time = millis();
}

void timeout_message()
{
	message_complete_callback();
}

void check_message_timeout()
{
	ulong time = millis();

	ulong diff = ulongDiff(time, delay_start_time);

	if (diff > delay_in_millis)
	{
		// timer has expired
		timeout_message();
	}
}

int number_being_input;
int number_being_input_upper_limit;
int number_being_input_lower_limit;

void(*number_complete_callback)(int result);

void complete_number_input()
{
	number_complete_callback(number_being_input);
}

void increment_number_value()
{
	TRACELN("Increment");
	number_being_input++;
	if (number_being_input > number_being_input_upper_limit)
	{
		number_being_input = number_being_input_lower_limit;
	}
	TRACELN(number_being_input);
	set_display_number_being_input(number_being_input);
}

void decrement_number_value()
{
	TRACELN("Decrement");
	number_being_input--;
	if (number_being_input < number_being_input_lower_limit)
	{
		number_being_input = number_being_input_upper_limit;
	}
	TRACELN(number_being_input);
	set_display_number_being_input(number_being_input);
}

void display_number_input(String in_number_input_prompt, int in_number_being_input,
	int in_number_being_input_lower_limit, int in_number_being_input_upper_limit, void(*callback)(int result))
{
	number_complete_callback = callback;

	set_number_input_display(in_number_input_prompt, in_number_being_input);

	number_being_input = in_number_being_input;
	number_being_input_upper_limit = in_number_being_input_upper_limit;
	number_being_input_lower_limit = in_number_being_input_lower_limit;
	menuState = readingNumberStateActive;
}

void do_back()
{
	TRACELN("Back called");
	exit_menu();
}

void get_number(String prompt, int start, int low, int high, void(*value_read)(int))
{
	peek_menu_stack_top()->active_menu_item = get_selected_item();
	display_number_input(prompt, start, low, high, value_read);
}

void display_message(String title, String message, int length, void(*completed)())
{
	peek_menu_stack_top()->active_menu_item = get_selected_item();
	display_message_screen(title, message, length, completed);
}

void message_done()
{
	Menu * next_menu = peek_menu_stack_top();
	set_menu_display(next_menu->menuText, next_menu->active_menu_item);
	menuState = mainMenuStateActive;
}

void dump_time()
{
	char time_buffer[100];
	read_time();
	sprintf(time_buffer, "%02d:%02d:%02d", pub_hour, pub_minute, pub_second);
	display_message("Time", time_buffer, 2000, message_done);
}

void dump_date()
{

	char date_buffer[100];
	read_time();
	sprintf(date_buffer, "%d/%d/%d", pub_day, pub_month, pub_year);
	display_message("Date", date_buffer, 2000, message_done);
}

void doneSetHours(int readHours)
{
	read_time();
	set_time(readHours, pub_minute, pub_second, pub_day, pub_month, pub_year);
	dump_time();
}

void setHours()
{
	TRACELN("Set hours called");
	get_number("Set hours", pub_hour, 0, 23, doneSetHours);
}

void doneSetMinutes(int readMins)
{
	read_time();
	set_time(pub_hour, readMins, pub_second, pub_day, pub_month, pub_year);
	dump_time();
}

void setMinutes()
{
	TRACELN("Set minutes called");
	get_number("Set minutes", pub_minute, 0, 59, doneSetMinutes);
}

void zeroSeconds()
{
	read_time();
	set_time(pub_hour, pub_minute, 0, pub_day, pub_month, pub_year);
	dump_time();
}

Menu timeMenu = { 0, "Set hours\nSet minutes\nZero seconds\nBack",{ setHours, setMinutes, zeroSeconds, do_back } };

//                          J	F	M	A	M	J	J	A	S	O	N	D  
int monthLengths[] = { 0,	31, 28, 31, 30, 31,	30,	31,	31,	30,	31,	30,	31 };

int getMonthLength(uint8_t month, uint16_t year)
{
	if (year % 4)
		return monthLengths[month];
	else
		return 29;
}

void setTime()
{
	TRACELN("Set date time called");
	enter_menu(&timeMenu);
}

void doneSetDays(int readDays)
{
	read_time();
	set_time(pub_hour, pub_minute, pub_second, readDays, pub_month, pub_year);
	dump_date();
}

void setDays()
{
	TRACELN("Set days called");
	int monthLength = getMonthLength(pub_month, pub_year);
	get_number("Set days", pub_day, 1, monthLength, doneSetDays);
}

void doneSetMonths(int readMonth)
{
	if (pub_day > getMonthLength(readMonth, pub_year))
	{
		display_message("Invalid Month", "Day not valid for this month", 2000, message_done);
	}
	else
	{
		set_time(pub_hour, pub_minute, pub_second, pub_day, readMonth, pub_year);
		dump_date();
	}
}

void setMonths()
{
	TRACELN("Set months called");
	get_number("Set month", pub_month, 1, 12, doneSetMonths);
}

void doneSetYears(int readYears)
{
	if (pub_day > getMonthLength(pub_month, readYears))
	{
		display_message("Invalid year", "Day not valid for this month", 2000, message_done);
	}
	else
	{
		set_time(pub_hour, pub_minute, pub_second, pub_day, pub_month, readYears);
		dump_date();
	}
}


void setYears()
{
	TRACELN("Set years called");
	get_number("Set year", pub_year, 2000, 3000, doneSetYears);
}

Menu dateMenu = { 0, "Set days\nSet months\nSet Years\nBack",{ setDays, setMonths, setYears, do_back } };


void setDate()
{
	enter_menu(&dateMenu);
}

void lora_send_done()
{
	TRACELN("LoRa send done");
	Menu * next_menu = peek_menu_stack_top();
	set_menu_display(next_menu->menuText, next_menu->active_menu_item);
	menuState = mainMenuStateActive;
}

void lora_send()
{
	TRACELN("LoRa send called");
	display_message("LoRa Send", "Sending", 2000, lora_send_done);
	send_to_lora();
}

void mqtt_send_done()
{
	TRACE("MQTT send done");
	Menu * next_menu = peek_menu_stack_top();
	set_menu_display(next_menu->menuText, next_menu->active_menu_item);
	menuState = mainMenuStateActive;
}

void mqtt_send()
{
	TRACELN("MQTT send");
	display_message("MQTT Send", "Sending", 2000, mqtt_send_done);
	pub_mqtt_force_send = true;
}

Menu mainMenu = { 0, "LoRa Send\nMQTT Send\nDate\nTime\nBack",{ lora_send, mqtt_send, setDate, setTime, do_back } };

void update_action(String title, String text)
{
	set_action_display(title, text);
}

MenuState stateBeforeAction;

void start_action(String title, String text)
{
	TRACELN("Start action");
	if (menuState == performingActionActive)
	{
		// If we try to start an action in the middle of one
		// just update the action
		update_action(title, text);
		return;
	}
	stateBeforeAction = menuState;
	set_action_display(title, text);
	menuState = performingActionActive;
}

void end_action()
{
	TRACELN("End action");
	TRACELN(stateBeforeAction);

	switch (stateBeforeAction)
	{
	case workingStateActive:
		set_working_state();
		break;
	case messageDisplayStateActive:
		activate_message_display();
		break;
	case mainMenuStateActive:
		if (!menu_stack_empty())
		{
			Menu * menu = peek_menu_stack_top();
			set_menu_display(menu->menuText, menu->active_menu_item);
		}
		break;
	case readingNumberStateActive:
		activate_number_input_display();
		break;
	case performingActionActive:
		// this should never happen
		break;
	}
	menuState = stateBeforeAction;
}

void error_stop(String title, String text)
{
	start_action(title, text);
	loop_lcd();
	while (true)
		delay(1);
}

void menu_up_pressed()
{
	switch (menuState)
	{
	case workingStateActive:
		break;
	case messageDisplayStateActive:
		break;
	case mainMenuStateActive:
		move_selector_up();
		break;
	case readingNumberStateActive:
		increment_number_value();
		break;
	case performingActionActive:
		// Up is presently disabled when performing an action
		break;
	}
}

void menu_down_pressed()
{
	switch (menuState)
	{
	case workingStateActive:
		break;
	case messageDisplayStateActive:
		break;
	case mainMenuStateActive:
		move_selector_down();
		break;
	case readingNumberStateActive:
		decrement_number_value();
		break;
	case performingActionActive:
		// Down is presently disabled when performing an action
		break;
	}
}

void menu_select_pressed()
{
	switch (menuState)
	{
	case workingStateActive:
		// Going into menu mode at the top level - 
		enter_menu(&mainMenu);
		break;
	case messageDisplayStateActive:
		break;
	case readingNumberStateActive:
		complete_number_input();
		break;
	case performingActionActive:
		// Enter is presently disabled when performing an action
		break;
	case mainMenuStateActive:
		uint8_t selected_item = get_selected_item();
		TRACE("selected:");
		TRACELN(selected_item);
		Menu * menu = peek_menu_stack_top();
		TRACELN(menu->menuText);
		menu->methods[selected_item]();
		break;
	}
}

// called if any of the display options change
// - only presently used by the remote commands that
// can download splash screen text that is displayed
// in workingStateActive

void refresh_menu ()
{
	switch (menuState)
	{
	case workingStateActive:
		set_working_display(); 
		break;
	case messageDisplayStateActive:
		break;
	case mainMenuStateActive:
		break;
	case readingNumberStateActive:
		break;
	case performingActionActive:
		break;
	}
}

void setup_menu()
{
	set_working_state();
}

void loop_menu()
{
	switch (menuState)
	{
	case workingStateActive:
		break;
	case messageDisplayStateActive:
		check_message_timeout();
		break;
	case mainMenuStateActive:
		break;
	case readingNumberStateActive:
		break;
	case performingActionActive:
		break;
	}
}

