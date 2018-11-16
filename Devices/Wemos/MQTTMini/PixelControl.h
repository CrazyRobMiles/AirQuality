#pragma once

#include <Adafruit_NeoPixel.h>

#define PIXELS 16
#define VERSION 1

#define NO_OF_VIRTUAL_PIXELS 12
#define VIRTUAL_PIXELS_LIMIT (NO_OF_VIRTUAL_PIXELS-1)

#define MAX_BRIGHTNESS 255

#define MILLIS_BETWEEN_UPDATES 20

#define NEOPIN 2

Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXELS, NEOPIN, NEO_GRB + NEO_KHZ800);

// All the brightness values are between 0 and 1
// Scale them for the particular display

float overall_brightness = 1.0;

struct Light_Factor
{
	float factor_value, max, min, update;
	void(*do_update) (Light_Factor * factor);
};

#define CLOSE_TOLERANCE 0.01

inline bool close_to(float a, float b)
{
	float diff = a - b;
	if (diff > 0)
	{
		if (diff > CLOSE_TOLERANCE)
			return false;
		else
			return true;
	}
	else
	{
		if (diff < -CLOSE_TOLERANCE)
			return false;
		else
			return true;
	}
}

void do_no_update(Light_Factor * target)
{
}


void do_update_larsen(Light_Factor * target)
{
	target->factor_value += target->update;
	if (target->factor_value > target->max)
	{
		target->factor_value = target->max;
		target->update = -target->update;
	}
	if (target->factor_value < target->min)
	{
		target->factor_value = target->min;
		target->update = -target->update;
	}
}

void do_update_loop(Light_Factor * target)
{
	target->factor_value += target->update;
	if (target->factor_value > target->max)
	{
		target->factor_value = target->min;
		return;
	}
	if (target->factor_value < target->min)
	{
		target->factor_value = target->max;
	}
}


void dump_light_factor(Light_Factor * factor)
{
	Serial.print("Value: ");
	Serial.print(factor->factor_value);
	Serial.print(" Max: ");
	Serial.print(factor->max);
	Serial.print(" Min: ");
	Serial.print(factor->min);
	Serial.print(" Update: ");
	Serial.print(factor->update);
}

#define RED_FACTOR 0
#define GREEN_FACTOR (RED_FACTOR+1)
#define BLUE_FACTOR (GREEN_FACTOR+1)
#define FLICKER_FACTOR (BLUE_FACTOR+1)
#define POSITION_FACTOR (FLICKER_FACTOR+1)
#define WIDTH_FACTOR (POSITION_FACTOR+1)
#define NO_OF_FACTORS (WIDTH_FACTOR+1)

char * factor_names[] = { "Red", "Green", "Blue", "Flicker", "Position", "Width" };

struct VirtualPixel
{
	Light_Factor factors[NO_OF_FACTORS];
};

struct VirtualPixel lamps[NO_OF_VIRTUAL_PIXELS];

void dumpVirtualPixel(VirtualPixel * lamp)
{
	for (int i = 0; i < NO_OF_FACTORS; i++)
	{
		Serial.print(factor_names[i]);
		Serial.print(": ");
		dump_light_factor(&lamp->factors[i]);
	}
}

void dumpVirtualPixels(struct VirtualPixel * lamps)
{
	for (int i = 0; i < NO_OF_VIRTUAL_PIXELS; i++)
	{
		dumpVirtualPixel(&lamps[i]);
	}
}

struct Pixel
{
	byte r, g, b;
};

struct Pixel pixels[PIXELS];

struct const_color {
	float r, g, b;
};

#define RED_PIXEL_COLOUR 0
#define GREEN_PIXEL_COLOUR 1
#define BLUE_PIXEL_COLOUR 2
#define YELLOW_PIXEL_COLOUR 3
#define MAGENTA_PIXEL_COLOUR 4
#define CYAN_PIXEL_COLOUR 5

struct const_color color_list[] =
{
	{1,0,0},  // RED
	{0,1,0},  // GREEN
	{0,0,1},  // BLUE
	{1,1,0},  // YELLOW
	{1,0,1},  // MAGENTA
	{0,1,1}   // CYAN
};

void clear_pixels()
{
	for (int i = 0; i < PIXELS; i++)
	{
		pixels[i].r = 0;
		pixels[i].g = 0;
		pixels[i].b = 0;
	}
}

byte clamp_colour(int c)
{
	if (c > 255)
		return 255;
	if (c < 0)
		return 0;
	return c;
}

void add_color_to_pixel(int pos, int r, int g, int b)
{
	byte newr = clamp_colour((int)pixels[pos].r + r);
	byte newg = clamp_colour((int)pixels[pos].g + g);
	byte newb = clamp_colour((int)pixels[pos].b + b);

	pixels[pos].r = newr;
	pixels[pos].g = newg;
	pixels[pos].b = newb;
}

void renderVirtualPixel(VirtualPixel * lamp)
{
	float brightness = lamp->factors[FLICKER_FACTOR].factor_value * MAX_BRIGHTNESS;

	// Map the position value from 360 degrees to a pixel number

	float pixel_pos = (lamp->factors[POSITION_FACTOR].factor_value / 360 * PIXELS);

	int pos = (int)(pixel_pos);

	float diff = pixel_pos - pos;

	float low_factor = 1 - diff;

	byte r = (byte)(lamp->factors[RED_FACTOR].factor_value * brightness * low_factor);
	byte g = (byte)(lamp->factors[GREEN_FACTOR].factor_value * brightness * low_factor);
	byte b = (byte)(lamp->factors[BLUE_FACTOR].factor_value * brightness * low_factor);

	add_color_to_pixel(pos, strip.gamma8(r), strip.gamma8(g), strip.gamma8(b));

	r = (byte)(lamp->factors[RED_FACTOR].factor_value * brightness * diff);
	g = (byte)(lamp->factors[GREEN_FACTOR].factor_value * brightness * diff);
	b = (byte)(lamp->factors[BLUE_FACTOR].factor_value * brightness * diff);

	add_color_to_pixel((pos + 1) % PIXELS, strip.gamma8(r), strip.gamma8(g), strip.gamma8(b));

}

void renderVirtualPixels(struct VirtualPixel * lamps)
{
	strip.clear();

	clear_pixels();

	for (int i = 0; i < NO_OF_VIRTUAL_PIXELS; i++)
	{
		renderVirtualPixel(&lamps[i]);
	}

	for (int i = 0; i < PIXELS; i++)
	{
		strip.setPixelColor(i, pixels[i].r, pixels[i].g, pixels[i].b);
	}

	strip.show();
}


void updateVirtualPixel(VirtualPixel * target)
{
	for (int i = 0; i < NO_OF_FACTORS; i++)
	{
		Light_Factor * factor = &target->factors[i];
		factor->do_update(factor);
	}
}

void updateVirtualPixels(struct VirtualPixel * lamps)
{
	for (int i = 0; i < NO_OF_VIRTUAL_PIXELS; i++)
	{
		updateVirtualPixel(&lamps[i]);
	}
	renderVirtualPixels(lamps);
}


void clear_factor(Light_Factor * target)
{
	target->factor_value = 0;
	target->max = 1.0;
	target->min = 0.0;
	target->update = 0.0;
	target->do_update = do_no_update;
}

void clearVirtualPixel(VirtualPixel * target)
{
	for (int i = 0; i < NO_OF_FACTORS; i++)
	{
		clear_factor(&target->factors[i]);
	}
}

void clearVirtualPixels(struct VirtualPixel * lamps)
{
	for (int i = 0; i < NO_OF_VIRTUAL_PIXELS; i++)
	{
		clearVirtualPixel(&lamps[i]);
	}
}

void setupVirtualPixel(VirtualPixel * target, float r, float g, float b, float pos, float width, float flicker)
{
	clearVirtualPixel(target);
	target->factors[RED_FACTOR].factor_value = r;
	target->factors[GREEN_FACTOR].factor_value = g;
	target->factors[BLUE_FACTOR].factor_value = b;
	target->factors[POSITION_FACTOR].factor_value = pos;
	target->factors[WIDTH_FACTOR].factor_value = width;
	target->factors[FLICKER_FACTOR].factor_value = flicker;
}

void setupVirtualPixelFactor(VirtualPixel * target, byte factor_number, float factor_value, float min, float max, float update, void(*do_update)(Light_Factor * f))
{
	target->factors[factor_number].factor_value = factor_value;
	target->factors[factor_number].max = max;
	target->factors[factor_number].min = min;
	target->factors[factor_number].update = update;
	target->factors[factor_number].do_update = do_update;
}

// If the pixels are illuminating a message these are the numbers of the 
// pixels that are actually in the message
// Need to change this is the message changes. 

int pixelsInMessage[] = { 0,1,2,3,4,5,6,7,8,9,10,11 };

void setupWalkingColour(const_color colour)
{
	byte color_pos;
	float start_speed = 0.5;
	float speed_update = 0.125;

	clearVirtualPixels(lamps);

	//  for (int i = 0; i < sizeof(pixelsInMessage)/sizeof(int); i++)	
	for (int i = 0; i < 8; i++)
	{
		setupVirtualPixel(&lamps[i], colour.r, colour.g, colour.b, pixelsInMessage[i], 0, 1.0);
		setupVirtualPixelFactor(&lamps[i], POSITION_FACTOR, pixelsInMessage[i], 0, 359, start_speed, do_update_loop);
		start_speed += speed_update;
	}
}

float calculateStepSize(float start, float end, int noOfSteps)
{
	float range = end - start;
	return range / noOfSteps;
}

void do_update_fade(Light_Factor * target)
{
	target->factor_value += target->update;
	if (close_to(target->factor_value, target->max))
	{
		target->factor_value = target->max;
		target->do_update = do_no_update;
	}
}

void startFade(Light_Factor * factor, float targetValue, int noOfsteps)
{
	factor->update = calculateStepSize(factor->factor_value, targetValue, noOfsteps);
	factor->max = targetValue;
	factor->do_update = do_update_fade;
}

void fadeWalkingColor(struct VirtualPixel * lamps, const_color newColour, int noOfSteps)
{
	for (int i = 0; i < NO_OF_VIRTUAL_PIXELS; i++)
	{
		startFade(&lamps[i].factors[RED_FACTOR], newColour.r, noOfSteps);
		startFade(&lamps[i].factors[GREEN_FACTOR], newColour.g, noOfSteps);
		startFade(&lamps[i].factors[BLUE_FACTOR], newColour.b, noOfSteps);
	}
}

void updateWalkingColor()
{
	updateVirtualPixels(lamps);
}

void updateReadingDisplay(float reading)
{
	//int airqLowLimit; 0 - airqLowWarnLimit
	//int airqLowWarnLimit; airQLowLimit - arqMidWarnLimit
	//int arqMidWarnLimit;  airqLowWarnLimit - airqHighWarnLimit
	//int airqHighWarnLimit; arqMidWarnLimit - airQHighAlertLimit
	//int airQHighAlertLimit; > airqHighWarnLimit

	if (reading < settings.airqLowLimit) {
		fadeWalkingColor(lamps, color_list[GREEN_PIXEL_COLOUR], 100);
		return;
	}

	if (reading < settings.airqLowWarnLimit) {
		fadeWalkingColor(lamps, color_list[YELLOW_PIXEL_COLOUR], 100);
		return;
	}

	if (reading < settings.airqMidWarnLimit) {
		fadeWalkingColor(lamps, color_list[MAGENTA_PIXEL_COLOUR], 100);
		return;
	}

	if (reading < settings.airqHighWarnLimit) {
		fadeWalkingColor(lamps, color_list[CYAN_PIXEL_COLOUR], 100);
		return;
	}

	fadeWalkingColor(lamps, color_list[RED_PIXEL_COLOUR], 100);
}

void setupWalkingColours()
{
	byte color_pos;
	float start_speed = 0.01;
	float speed_update = 0.005;

	clearVirtualPixels(lamps);

	for (int i = 0; i < 8; i++)
	{
		int cn = i % (sizeof(color_list) / sizeof(const_color));
		setupVirtualPixel(&lamps[i], color_list[cn].r, color_list[cn].g, color_list[cn].b, pixelsInMessage[i], 0, 1.0);
		setupVirtualPixelFactor(&lamps[i], POSITION_FACTOR, pixelsInMessage[i], 0, 359, start_speed, do_update_loop);
		start_speed += speed_update;
	}
}


void setup_text_message(int r, int g, int b)
{
	clearVirtualPixels(lamps);

	for (int i = 0; i < sizeof(pixelsInMessage) / sizeof(int); i++)
	{
		setupVirtualPixel(&lamps[i], r, g, b, pixelsInMessage[i], 0, 1.0);
		setupVirtualPixelFactor(&lamps[i], FLICKER_FACTOR, 0, 0.5, 1.0, 0.01, do_update_larsen);
	}
}

void(*pixel_animation_update)();

int lastColour = -1;

void handleTransition(int targetColor)
{
	if (lastColour != targetColor)
	{
		fadeWalkingColor(lamps, color_list[targetColor], 100);
		lastColour = targetColor;
	}
}

struct Animation_item
{
	void(*animation_setup) ();
	void(*animation_update)();
	int duration;
};

void setup_coloured_message()
{

}

void update_coloured_message()
{
	updateVirtualPixels(lamps);
}


void setup_walking_colours()
{

}

void update_walking_colours()
{
	updateVirtualPixels(lamps);
}


struct Animation_item animations[] = {
	{setup_coloured_message, update_coloured_message,  600 },
	{setup_walking_colours, update_walking_colours, 600} };

int animation_number = 0;
int tick_count = 0;

unsigned long millisOfLastUpdate;

void setup_pixels()
{
	strip.begin();
	strip.show(); // Initialize all pixels to 'off'
	clearVirtualPixels(lamps);
	setupWalkingColour({ 0,1,0 });
	millisOfLastUpdate = millis();
	pixel_animation_update = updateWalkingColor;
}

void loop_pixels()
{
	unsigned long currentMillis = millis();
	unsigned long millisSinceLastUpdate = ulongDiff(currentMillis, millisOfLastUpdate);
	if (millisSinceLastUpdate >= MILLIS_BETWEEN_UPDATES) {
		pixel_animation_update();
		millisOfLastUpdate = currentMillis;
	}
}


