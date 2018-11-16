// This example just provide basic function test;
// For more informations, please vist www.heltec.cn or mail to support@heltec.cn

#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306.h" // alias for #include "SSD1306Wire.h"`

#include "OLEDDisplayUi.h"
#include "images.h"

//OLED pins to ESP32 GPIOs via this connecthin:
//OLED_SDA -- GPIO4
//OLED_SCL -- GPIO15
//OLED_RST -- GPIO16

// later version of display constructor nominates a pin for reset which must be added to the
// constructor call

SSD1306  display(0x3c, 4, 15, 16);

OLEDDisplayUi ui     ( &display );

void clockOverlay(OLEDDisplay *display, OLEDDisplayUiState* state) {
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  char time_buffer[100];
  sprintf(time_buffer, "%02d:%02d", pub_hour, pub_minute);
  display->setFont(ArialMT_Plain_10);
  display->drawString(128, 0, time_buffer);
}

void drawSplashScreen(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  display->setTextAlignment(TEXT_ALIGN_CENTER);

  display->setFont(ArialMT_Plain_16);
  display->drawString(64 + x, 10 + y,settings.splash_screen_top_line);

  display->setFont(ArialMT_Plain_16);
  display->drawString(64 + x, 30 + y, settings.splash_screen_bottom_line);
}

void drawAirQuality(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  char number_buffer [100];
  
  display->setTextAlignment(TEXT_ALIGN_LEFT);

  display->setFont(ArialMT_Plain_16);
  sprintf(number_buffer, "PM10:  %.1f", pub_ppm_10); 
  display->drawString(0 + x, 10 + y, number_buffer);

  sprintf(number_buffer, "PPM25:  %.1f", pub_ppm_25); 
  display->drawString(0 + x, 30 + y, number_buffer);

}

void drawTemp(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
	char number_buffer[100];

	display->setTextAlignment(TEXT_ALIGN_LEFT);

	display->setFont(ArialMT_Plain_16);
	sprintf(number_buffer, "Temp:  %.1f", pub_temp);
	display->drawString(0 + x, 0 + y, number_buffer);

	sprintf(number_buffer, "Pressure:  %.1f", pub_pressure);
	display->drawString(0 + x, 18 + y, number_buffer);

	sprintf(number_buffer, "Humidity:  %.1f", pub_humidity);
	display->drawString(0 + x, 36 + y, number_buffer);

}

void drawFrame4(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  // Demo for drawStringMaxWidth:
  // with the third parameter you can define the width after which words will be wrapped.
  // Currently only spaces and "-" are allowed for wrapping
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawStringMaxWidth(0 + x, 10 + y, 128, "Lorem ipsum\n dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore.");
}

void drawFrame5(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {

}

#define MAX_LINE_LENGTH 40
#define MAX_LINE_LIMIT MAX_LINE_LENGTH-1
#define NO_OF_LINES 5

char active_menu[NO_OF_LINES][MAX_LINE_LENGTH];

uint8_t active_menu_item = 0;
uint8_t no_of_menu_items = 0;

void drawMenuFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {

	uint8_t lineHeight = 12;
	uint8_t linePos = 0;
	display->setTextAlignment(TEXT_ALIGN_LEFT);

	display->setFont(ArialMT_Plain_16);

	for (int i = 0; i < no_of_menu_items; i++)
	{
		if (i == active_menu_item)
		{
			display->drawString(0 + x, linePos + y, "=>");
		}
		display->drawString(20 + x, linePos + y, active_menu[i]);
		linePos += lineHeight;
	}
}

void build_menu(String menu_text)
{
	uint8_t line_count = 0;
	uint8_t line_pos = 0;
	uint8_t input_pos = 0;
	while (true)
	{
		if (menu_text[input_pos] == '\n')
		{
			// terminate the current string
			active_menu[line_count][line_pos] = 0;
			// Skip past the line feed
			input_pos++;
			// move on to the next line
			line_count++;
			line_pos = 0;
			if (line_count == NO_OF_LINES)
				break;
		}

		if (menu_text[input_pos] == 0)
		{
			// terminate the current string
			active_menu[line_count][line_pos] = 0;
			line_count++;
			break;
		}

		if (line_pos == MAX_LINE_LIMIT)
		{
			// line to long for display - truncate - spin to end of line
			while (menu_text[input_pos] != 0 && menu_text[input_pos] != '\n')
				input_pos++;
			break;
		}

		active_menu[line_count][line_pos] = menu_text[input_pos];
		input_pos++;
		line_pos++;
	}
	no_of_menu_items = line_count;
}

// This array keeps function pointers to all frames
// frames are the single views that slide in

FrameCallback workingFrames[] = { drawSplashScreen, drawAirQuality, drawTemp };
FrameCallback workingFramesNoSplash[] = { drawAirQuality, drawTemp };

// how many frames are there?
int workingFrameCount = 3;

// Overlays are statically drawn on top of a frame eg. a clock
OverlayCallback workingOverlays[] = { clockOverlay };
int workingOverlaysCount = 1;

void set_working_display()
{
	// Customize the active and inactive symbol
	ui.setActiveSymbol(activeSymbol);
	ui.setInactiveSymbol(inactiveSymbol);

	ui.enableAllIndicators();
	// You can change this to
	// TOP, LEFT, BOTTOM, RIGHT
	ui.setIndicatorPosition(BOTTOM);

	// Defines where the first frame is located in the bar.
	ui.setIndicatorDirection(LEFT_RIGHT);

	// You can change the transition that is used
	// SLIDE_LEFT, SLIDE_RIGHT, SLIDE_UP, SLIDE_DOWN
	ui.setFrameAnimation(SLIDE_LEFT);

	// Add frames
	if((settings.splash_screen_bottom_line[0]==0) && (settings.splash_screen_top_line[0]==0))
	{
		// If the splash screen text is empty - don't display the splash screen
		ui.setFrames(workingFramesNoSplash, workingFrameCount-1);
	}
	else
	{
		ui.setFrames(workingFrames, workingFrameCount);
	}

	// Add overlays
	ui.setOverlays(workingOverlays, workingOverlaysCount);

	ui.enableAutoTransition();
}

String message_display_text;
String message_display_title;

void drawMessageFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
	display->setTextAlignment(TEXT_ALIGN_CENTER);

	display->setFont(ArialMT_Plain_16);
	display->drawString(64 + x, 0 + y, message_display_title);

	display->setFont(ArialMT_Plain_16);
	display->drawStringMaxWidth(64 + x, 20 + y, 128, message_display_text);
}


FrameCallback messageFrames[] = { drawMessageFrame };

// how many frames are there?
int messageFrameCount = 1;

// Overlays are statically drawn on top of a frame eg. a clock
OverlayCallback messageOverlays[] = { clockOverlay };
int messageOverlayCount = 0;

void activate_message_display()
{
	TRACELN("Set message display");

	ui.disableAllIndicators();
	ui.disableAutoTransition();

	// Add frames
	ui.setFrames(messageFrames, messageFrameCount);

	// Add overlays
	ui.setOverlays(messageOverlays, messageOverlayCount);
}


void set_message_display(String title, String text)
{
	TRACELN("Set message display");
	message_display_title = title;
	message_display_text = text;

	activate_message_display();
}




String action_display_text;
String action_display_title;

void drawActionFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
	display->setTextAlignment(TEXT_ALIGN_CENTER);

	display->setFont(ArialMT_Plain_16);
	display->drawString(64 + x, 0 + y, action_display_title);

	display->setFont(ArialMT_Plain_16);
	display->drawStringMaxWidth(64 + x, 20 + y, 128, action_display_text);
}


FrameCallback actionFrames[] = { drawActionFrame };

// how many frames are there?
int actionFrameCount = 1;

// Overlays are statically drawn on top of a frame eg. a clock
OverlayCallback actionOverlays[] = { clockOverlay };
int actionOverlayCount = 0;

void set_action_display(String title, String text)
{
	TRACELN("Set message display");
	action_display_title = title;
	action_display_text = text;

	ui.disableAllIndicators();
	ui.disableAutoTransition();

	// Add frames
	ui.setFrames(actionFrames, actionFrameCount);

	// Add overlays
	ui.setOverlays(actionOverlays, actionOverlayCount);
}


int display_number_being_input;
String number_input_prompt;

void drawNumberInputFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {

	display->setTextAlignment(TEXT_ALIGN_CENTER);

	display->setFont(ArialMT_Plain_16);
	display->drawString(64 + x, 0 + y, number_input_prompt);

	display->setFont(ArialMT_Plain_16);
	char buffer[20];
	sprintf(buffer, "%d", display_number_being_input);

	display->drawString(64 + x, 40 + y, buffer);
}

void set_display_number_being_input(int new_value)
{
	display_number_being_input = new_value;
}

FrameCallback numberInputFrames[] = { drawNumberInputFrame };

// how many frames are there?
int numberInputFrameCount = 1;

// Overlays are statically drawn on top of a frame eg. a clock
OverlayCallback numberInputOverlays[] = { clockOverlay };
int numberInputOverlayCount = 0;

void activate_number_input_display()
{
	ui.disableAllIndicators();
	ui.disableAutoTransition();

	// Add frames
	ui.setFrames(numberInputFrames, numberInputFrameCount);

	// Add overlays
	ui.setOverlays(numberInputOverlays, numberInputOverlayCount);
}

void set_number_input_display(String in_number_input_prompt, int in_number_being_input)
{
	TRACELN("Set number input display");
	number_input_prompt = in_number_input_prompt;
	display_number_being_input = in_number_being_input;
	activate_number_input_display();
}

FrameCallback menuFrames[] = {drawMenuFrame};

// how many frames are there?
int menuFrameCount = 1;

// Overlays are statically drawn on top of a frame eg. a clock
OverlayCallback menuOverlays[] = { clockOverlay };
int menuOverlayCount = 0;

// Used to connect the menu to the LCD

// Displays the menu text and sets the cursor positon to the given 
// item. 

void set_menu_display(String menu_text, uint64_t pos_in_menu)
{
	build_menu(menu_text);
	active_menu_item = pos_in_menu;

	ui.disableAllIndicators();
	ui.disableAutoTransition();

	// Add frames
	ui.setFrames(menuFrames, menuFrameCount);

	// Add overlays
	ui.setOverlays(menuOverlays, menuOverlayCount);
}


void move_selector_up()
{
	TRACELN("Moving up");
	if (active_menu_item == 0)
	{
		TRACELN("Up wrapping");
		active_menu_item = no_of_menu_items - 1;
	}
	else
		active_menu_item--;
}

void move_selector_down()
{
	TRACELN("Moving down");
	active_menu_item++;

	if (active_menu_item == no_of_menu_items)
	{
		TRACELN("Down wrapping");
		active_menu_item = 0;
	}
}

int get_selected_item()
{
	return active_menu_item;
}

void setup_lcd() {
 #ifdef VERBOSE
 Serial.println("Setting up LCD");
 #endif
  
  // The ESP is capable of rendering 60fps in 80Mhz mode
  // but that won't give you much time for anything else
  // run it in 160Mhz mode or just set it to 30 fps
  ui.setTargetFPS(60);

  set_working_display();

  // Initialising the UI will init the display too.
  ui.init();

  //display.flipScreenVertically();

}

void loop_lcd() {
  int remainingTimeBudget = ui.update();

  if (remainingTimeBudget > 0) {
    // You can do some work here
    // Don't do stuff if you are below your
    // time budget.
    delay(remainingTimeBudget);
  }
}
