#pragma once

void start_action(String title, String text)
{
	Serial.print(title);
	Serial.print(" ");
	Serial.println(text);
}

void update_action(String title, String text)
{
	Serial.print(title);
	Serial.print(" ");
	Serial.println(text);
}

void end_action()
{
	TRACE("End action");
}