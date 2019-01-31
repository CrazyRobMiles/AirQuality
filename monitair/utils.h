#pragma once

unsigned long ulongDiff(unsigned long end, unsigned long start)
{
	if (end >= start)
	{
		return end - start;
	}
	else
	{
		return ULONG_MAX - start + end + 1;
	}
}

#define MESSAGE_BUFFER_SIZE 200
char messageBuffer[MESSAGE_BUFFER_SIZE];

struct ColourValue {
	float r, g, b;
};

bool coloursEqual(ColourValue a, ColourValue b)
{
	if (a.r != b.r) return false;
	if (a.g != b.g) return false;
	if (a.b != b.b) return false;
	return true;
}

#define BLACK_PIXEL_COLOUR {0,0,0}
#define RED_PIXEL_COLOUR {1,0,0}
#define GREEN_PIXEL_COLOUR {0,1,0}
#define BLUE_PIXEL_COLOUR {0,0,1}
#define YELLOW_PIXEL_COLOUR {1,1,0}
#define MAGENTA_PIXEL_COLOUR {1,0,1}
#define CYAN_PIXEL_COLOUR {0,1,1}
#define WHITE_PIXEL_COLOUR {1,1,1}

// defined in pixels.h

void beginStatusDisplay();
boolean addStatusItem(boolean status);

