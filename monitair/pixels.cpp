#include <Arduino.h>
#include <NeoPixelBus.h>
#include <NeoPixelBrightnessBus.h>
#include <NeoPixelAnimator.h>

#include "pixels.h"
#include "settings.h"
#include "airquality.h"


NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart800KbpsMethod> * strip;

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

boolean coloursEqual(ColourValue a, ColourValue b)
{
	if (!close_to(a.r,b.r)) return false;
	if (!close_to(a.g,b.g)) return false;
	if (!close_to(a.b,b.b)) return false;
	return true;
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

struct Pixel pixels[MAX_NO_OF_PIXELS];

// ColourValue struct is defined in utils.h

struct ColourValue color_list[] =
{
	RED_PIXEL_COLOUR,
	GREEN_PIXEL_COLOUR,
	BLUE_PIXEL_COLOUR,
	YELLOW_PIXEL_COLOUR,
	MAGENTA_PIXEL_COLOUR,
	CYAN_PIXEL_COLOUR,
	WHITE_PIXEL_COLOUR
};

void clear_pixels()
{
	for (int i = 0; i < settings.noOfPixels; i++)
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

	float pixel_pos = (lamp->factors[POSITION_FACTOR].factor_value / 360 * settings.noOfPixels);

	int pos = (int)(pixel_pos);

	float diff = pixel_pos - pos;

	float low_factor = 1 - diff;

	byte r = (byte)(lamp->factors[RED_FACTOR].factor_value * brightness * low_factor);
	byte g = (byte)(lamp->factors[GREEN_FACTOR].factor_value * brightness * low_factor);
	byte b = (byte)(lamp->factors[BLUE_FACTOR].factor_value * brightness * low_factor);

	add_color_to_pixel(pos, r, g, b);

	r = (byte)(lamp->factors[RED_FACTOR].factor_value * brightness * diff);
	g = (byte)(lamp->factors[GREEN_FACTOR].factor_value * brightness * diff);
	b = (byte)(lamp->factors[BLUE_FACTOR].factor_value * brightness * diff);

	add_color_to_pixel((pos + 1) % settings.noOfPixels, r, g, b);

}

void renderVirtualPixels(struct VirtualPixel * lamps)
{
	clear_pixels();

	for (int i = 0; i < NO_OF_VIRTUAL_PIXELS; i++)
	{
		renderVirtualPixel(&lamps[i]);
	}

	for (int i = 0; i < settings.noOfPixels; i++)
	{
		strip->SetPixelColor(i, { pixels[i].r,pixels[i].g,pixels[i].b });
	}

	strip->Show();
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

void setupWalkingColour(ColourValue colour)
{
	byte color_pos;
	float start_speed = 0.5;
	float speed_update = 0.125;

	float degreesPerPixel = 360 / settings.noOfPixels;

	clearVirtualPixels(lamps);

	//  for (int i = 0; i < sizeof(pixelsInMessage)/sizeof(int); i++)	
	for (int i = 0; i < 8; i++)
	{
		setupVirtualPixel(&lamps[i], colour.r, colour.g, colour.b, i*degreesPerPixel, 0, 1.0);
		setupVirtualPixelFactor(&lamps[i], POSITION_FACTOR, i, 0, 359, start_speed, do_update_loop);
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
	if (close_to(factor->factor_value, targetValue))
		return;

	factor->update = calculateStepSize(factor->factor_value, targetValue, noOfsteps);
	factor->max = targetValue;
	factor->do_update = do_update_fade;
}

void fadeWalkingColour(struct VirtualPixel * lamps, ColourValue newColour, int noOfSteps)
{
	for (int i = 0; i < NO_OF_VIRTUAL_PIXELS; i++)
	{
		startFade(&lamps[i].factors[RED_FACTOR], newColour.r, noOfSteps);
		startFade(&lamps[i].factors[GREEN_FACTOR], newColour.g, noOfSteps);
		startFade(&lamps[i].factors[BLUE_FACTOR], newColour.b, noOfSteps);
	}
}

void fadeWalkingColours(struct VirtualPixel * lamps, ColourValue * newColours, int noOfColours, int noOfSteps)
{
	for (int i = 0; i < NO_OF_VIRTUAL_PIXELS; i++)
	{
		int colourNo = i % noOfColours;
		startFade(&lamps[i].factors[RED_FACTOR], newColours[colourNo].r, noOfSteps);
		startFade(&lamps[i].factors[GREEN_FACTOR], newColours[colourNo].g, noOfSteps);
		startFade(&lamps[i].factors[BLUE_FACTOR], newColours[colourNo].b, noOfSteps);
	}
}

enum readingDisplayStates { veryLow, lowWarn, lowMid, lowHigh, highHigh, alert, sensorFailed, noAverage };

readingDisplayStates displayState;

readingDisplayStates getDisplayStateFromValue(float value)
{
	if (value < settings.airqLowLimit)
		return veryLow;

	if (value < settings.airqLowWarnLimit)
		return lowWarn;

	if (value < settings.airqMidWarnLimit)
		return lowMid;

	if (value < settings.airqHighWarnLimit)
		return lowHigh;

	if (value < settings.airqHighAlertLimit)
		return highHigh;

	return alert;
}

struct ColourValue badAirQReadingColours[] = { BLUE_PIXEL_COLOUR, RED_PIXEL_COLOUR };
struct ColourValue noAverageAirQReadingColours[] = { YELLOW_PIXEL_COLOUR, GREEN_PIXEL_COLOUR };

void setDisplayState(readingDisplayStates newState)
{
	displayState = newState;

	switch (displayState)
	{
	case veryLow:
		fadeWalkingColour(lamps, GREEN_PIXEL_COLOUR, 100);
		break;
	case lowWarn:
		fadeWalkingColour(lamps, YELLOW_PIXEL_COLOUR, 100);
		break;
	case lowMid:
		fadeWalkingColour(lamps, ORANGE_PIXEL_COLOUR, 100);
		break;
	case lowHigh:
		fadeWalkingColour(lamps, RED_PIXEL_COLOUR, 100);
		break;
	case highHigh:
		fadeWalkingColour(lamps, MAGENTA_PIXEL_COLOUR, 100);
		break;
	case alert:
		fadeWalkingColour(lamps, WHITE_PIXEL_COLOUR, 100);
		break;
	case sensorFailed:
		fadeWalkingColours(lamps, badAirQReadingColours, sizeof(badAirQReadingColours) / sizeof(struct ColourValue),
			100);
		break;
	case noAverage:
		fadeWalkingColours(lamps, noAverageAirQReadingColours, sizeof(noAverageAirQReadingColours) / sizeof(struct ColourValue),
			100);
		break;
	}
}

struct sensor  * sourceAirqSensor = NULL;
int previousAirqSensorStatus = -1;

void updateReadingDisplay()
{
	if (sourceAirqSensor == NULL)
	{
			sourceAirqSensor = findSensorByName("Air quality");
	}

	readingDisplayStates newDisplayState;

	if (sourceAirqSensor->status == SENSOR_OK)
	{
		airqualityReading * sourceAirqReading = 
			(airqualityReading *) sourceAirqSensor->activeReading;

		unsigned long millisSinceAverage = ulongDiff(millis(), sourceAirqReading->lastAirqAverageMillis);

		if (millisSinceAverage < AIRQ_AVERAGE_LIFETIME_MSECS)
		{
			newDisplayState = getDisplayStateFromValue(sourceAirqReading->pm25Average);
		}
		else
		{
			newDisplayState = noAverage;
		}
	}
	else
	{
		newDisplayState = sensorFailed;
	}

	if (newDisplayState == displayState)
		return;

	setDisplayState(newDisplayState);
}

void setupWalkingMultipleColours()
{
	byte color_pos;
	float start_speed = 0.01;
	float speed_update = 0.005;

	clearVirtualPixels(lamps);

	for (int i = 0; i < 8; i++)
	{
		int cn = i % (sizeof(color_list) / sizeof(ColourValue));
		setupVirtualPixel(&lamps[i], color_list[cn].r, color_list[cn].g, color_list[cn].b, i, 0, 1.0);
		setupVirtualPixelFactor(&lamps[i], POSITION_FACTOR, i, 0, 359, start_speed, do_update_loop);
		start_speed += speed_update;
	}
}

int lastColour = -1;

void handleTransition(int targetColor)
{
	if (lastColour != targetColor)
	{
		fadeWalkingColour(lamps, color_list[targetColor], 100);
		lastColour = targetColor;
	}
}

unsigned long millisOfLastPixelUpdate;

void startPixelStrip()
{
	if (strip != NULL)
		return;

	strip = new NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart800KbpsMethod> (12, settings.pixelControlPinNo);
	strip->Begin();
	strip->SetPixelColor(0, { 255,0,0 });
	strip->Show();
}

// status display doesn't use the animated leds
// this means that it can overlay the display 

int statusPixelNo = 0;

void initialiseStatusDisplay(byte r, byte g, byte b)
{
	statusPixelNo = 0;

	startPixelStrip();

	for (int i = 0; i < settings.noOfPixels; i++)
	{
		strip->SetPixelColor(i, { r,g,b });
	}
}


boolean setStatusDisplayPixel(int pixelNumber, boolean statusOK)
{
	if (pixelNumber >= settings.noOfPixels)
		return false;

	if (statusOK)
	{
		strip->SetPixelColor(pixelNumber, { 0,255,0 });
	}
	else
	{
		strip->SetPixelColor(pixelNumber, { 255,0,0 });
	}

	return true;
}


void beginStatusDisplay()
{
	initialiseStatusDisplay(0, 0, 10);
}

void beginWifiStatusDisplay()
{
	initialiseStatusDisplay(10, 0, 10);
}

void renderStatusDisplay()
{
	strip->Show();
}

boolean addStatusItem(boolean status)
{
	if (statusPixelNo >= settings.noOfPixels)
		return false;

	setStatusDisplayPixel(statusPixelNo, status);

	statusPixelNo++;

	return true;
}

int startPixel(struct process * pixelProcess)
{
	if (settings.noOfPixels == 0)
	{
		pixelProcess->status = PIXEL_ERROR_NO_PIXELS;
		return PIXEL_ERROR_NO_PIXELS;
	}

	startPixelStrip();

	clearVirtualPixels(lamps);
	millisOfLastPixelUpdate = millis();

	pixelProcess->status = PIXEL_OK;
	return PIXEL_OK;
}

void showDeviceStatus();  // declared in control.h
boolean getInputSwitchValue(); // declared in inputswitch.h

int updatePixel(struct process * pixelProcess)
{
	if (pixelProcess->status != PIXEL_OK)
	{
		return pixelProcess->status;
	}

	unsigned long currentMillis = millis();
	unsigned long millisSinceLastUpdate = ulongDiff(currentMillis, millisOfLastPixelUpdate);
	if (millisSinceLastUpdate >= MILLIS_BETWEEN_UPDATES) {
		if (getInputSwitchValue())
		{
			showDeviceStatus();
		}
		else
		{
			updateReadingDisplay();
			updateVirtualPixels(lamps);
		}
		millisOfLastPixelUpdate = currentMillis;
	}

	return pixelProcess->status;
}

int stopPixel(struct process * pixelProcess)
{
	pixelProcess->status = PIXEL_OFF;
	return PIXEL_OFF;
}

void pixelStatusMessage(struct process * pixelProcess, char * buffer, int bufferLength)
{
	switch (pixelProcess->status)
	{
	case PIXEL_OK:
		snprintf(buffer, bufferLength, "PIXEL OK");
		break;
	case PIXEL_OFF:
		snprintf(buffer, bufferLength, "PIXEL OFF");
		break;
	default:
		snprintf(buffer, bufferLength, "Pixel status invalid");
		break;
	}
}


