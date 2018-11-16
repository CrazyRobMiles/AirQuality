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
