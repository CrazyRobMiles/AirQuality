#pragma once

#include "utils.h"
#include "processes.h"

#define PIXEL_OK 0
#define PIXEL_OFF 1
#define PIXEL_ERROR_NO_PIXELS -1

#define NO_OF_VIRTUAL_PIXELS 12

#define MAX_NO_OF_PIXELS 12

#define MILLIS_BETWEEN_UPDATES 20

#define MAX_BRIGHTNESS 255

struct ColourValue {
	float r, g, b;
};

#define BLACK_PIXEL_COLOUR {0,0,0}
#define RED_PIXEL_COLOUR {1,0,0}
#define GREEN_PIXEL_COLOUR {0,1,0}
#define BLUE_PIXEL_COLOUR {0,0,1}
#define YELLOW_PIXEL_COLOUR {1,1,0}
#define ORANGE_PIXEL_COLOUR {1,165.0/255.0,0}
#define MAGENTA_PIXEL_COLOUR {1,0,1}
#define CYAN_PIXEL_COLOUR {0,1,1}
#define WHITE_PIXEL_COLOUR {1,1,1}

boolean coloursEqual(ColourValue a, ColourValue b);

void beginStatusDisplay();
void beginWifiStatusDisplay();
boolean addStatusItem(boolean status);
void renderStatusDisplay();
void setupWalkingColour(ColourValue colour);

int startPixel(struct process * pixelProcess);
int updatePixel(struct process * pixelProcess);
int stopPixel(struct process * pixelProcess);
void pixelStatusMessage(struct process * pixelProcess, char * buffer, int bufferLength);



