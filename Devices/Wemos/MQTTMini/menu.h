#pragma once


void start_action(String title, String text)
{
	TRACE("Start action: ");
	TRACE(title);
	TRACE("  ");
	TRACELN(text);
}

void update_action(String title, String text)
{
	TRACE("Update action: ");
	TRACE(title);
	TRACE("  ");
		TRACELN(text);
}

void end_action()
{
	TRACE("End action");
}