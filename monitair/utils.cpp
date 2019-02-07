#include <Arduino.h>
#include <limits.h>

#include "utils.h"

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