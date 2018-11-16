#pragma once
#include "WiFiConnection.h"

char * dayNames[] = { "", "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

char * monthNames[] = { "", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

enum ClockState { CLockAwaitingWiFi, ClockAwaitingSync, ClockRunning};

ClockState clockState;

#define NTP_PACKET_SIZE			48
#define NTP_LOCAL_PORT			4242
#define NTP_SERVER				"pool.ntp.org"
#define NTP_TIMEOUT				1500			// milliseconds
#define NTP_INTERVAL			600				// default update interval in seconds
#define NTP_RETRY				5				// Retry after this many seconds on failed NTP
#define NTP_STALE_AFTER			3600			// If update due for this many seconds, set timeStatus to timeNeedsSync

void setup_clock()
{
#ifdef DEBUG
	setDebug(INFO);
#endif
	pub_clock_valid = false;
	clockState = CLockAwaitingWiFi;
}

#define CLOCK_SYNC_TIMEOUT 5

void do_clock_update()
{
	events();

	switch (timeStatus())
	{
	case timeNotSet:
		pub_clock_valid = false;
		clockState = ClockAwaitingSync;
		break;
	case timeSet:
		pub_clock_valid = true;
		pub_hour = hour();
		pub_minute = minute();
		pub_second = second();
		pub_day = day();
		pub_month = month();
		pub_year = year();
		pub_day_of_week = weekday();
		break;
	case timeNeedsSync:
		pub_clock_valid = false;
		clockState = ClockAwaitingSync;
		break;
	}
}

void loop_clock()
{
	switch (clockState)
	{
	case CLockAwaitingWiFi:
		if (wifiState == WiFiConnected)
		{
			clockState = ClockAwaitingSync;
		}
		break;
	case ClockAwaitingSync:
		TRACELN("Waiting for clock sync");
		if (waitForSync(CLOCK_SYNC_TIMEOUT)) {
			clockState = ClockRunning;
			TRACELN("Clock sync complete");
		}
		break;

	case ClockRunning:
		do_clock_update();
		break;
	}
}