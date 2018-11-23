#pragma once

#include "utils.h"
#include "commands.h"

unsigned long mqtt_reading_retry_interval_in_millis;
unsigned long mqtt_reading_interval_in_millis;
unsigned long milliseconds_at_last_mqtt_update;

unsigned long lora_reading_retry_interval_in_millis;
unsigned long lora_reading_interval_in_millis;
unsigned long milliseconds_at_last_lora_update;

// return true if it is time for an update
bool mqtt_interval_has_expired()
{
	unsigned long time_in_millis = millis();
	unsigned long millis_since_last_mqtt_update = ulongDiff(time_in_millis, milliseconds_at_last_mqtt_update);


	if (millis_since_last_mqtt_update > mqtt_reading_interval_in_millis)
	{
		return true; 
	}
	return false;
}


bool lora_interval_has_expired()
{
	unsigned long time_in_millis = millis();
	unsigned long millis_since_last_lora_update = ulongDiff(time_in_millis, milliseconds_at_last_lora_update);


	if (millis_since_last_lora_update > lora_reading_interval_in_millis)
	{
		return true;  
	}
	return false;
}


unsigned long time_to_next_mqtt_update()
{
	unsigned long millis_since_last_mqtt_update = ulongDiff(millis(), milliseconds_at_last_mqtt_update);

	if (millis_since_last_mqtt_update > mqtt_reading_interval_in_millis)
		return 0;
	else
		return mqtt_reading_interval_in_millis - millis_since_last_mqtt_update;
}

unsigned long time_to_next_lora_update()
{
	unsigned long millis_since_last_lora_update = ulongDiff(millis(), milliseconds_at_last_lora_update);

	if (millis_since_last_lora_update > lora_reading_interval_in_millis)
		return 0;
	else
		return lora_reading_interval_in_millis - millis_since_last_lora_update;
}

void readings_ready()
{
	unsigned long time_in_millis = millis();

	if (settings.mqtt_enabled)
	{
		unsigned long millis_since_last_mqtt_update = ulongDiff(time_in_millis, milliseconds_at_last_mqtt_update);


		if (millis_since_last_mqtt_update > mqtt_reading_interval_in_millis)
		{
			milliseconds_at_last_mqtt_update = time_in_millis;

			if (send_to_mqtt())
			{
				mqtt_reading_interval_in_millis = settings.seconds_per_mqtt_update * 1000;
			}
			else
			{
				mqtt_reading_interval_in_millis = settings.seconds_per_mqtt_retry * 1000;
			}
		}
	}

	if (pub_mqtt_force_send)
	{
		if (send_to_mqtt())
		{
			pub_mqtt_force_send = false;
		}
	}

	if (settings.lora_enabled)
	{
		unsigned long millis_since_last_lora_update = ulongDiff(time_in_millis, milliseconds_at_last_lora_update);

		if (millis_since_last_lora_update > lora_reading_interval_in_millis)
		{
			milliseconds_at_last_lora_update = time_in_millis ;
			send_to_lora();
		}
	}

	if (pub_lora_force_send)
	{
		if (send_to_lora())
		{
			pub_lora_force_send = false;
		}
	}
}


void setup_timing()
{
	unsigned long time_in_millis = millis();
	mqtt_reading_interval_in_millis = settings.seconds_per_mqtt_update * 1000;
	milliseconds_at_last_mqtt_update = time_in_millis;
	lora_reading_interval_in_millis = settings.seconds_per_lora_update * 1000;
	milliseconds_at_last_lora_update = time_in_millis;
}

void loop_timing()
{
	if (pub_air_values_ready && pub_bme_values_ready)
	{
		readings_ready();
		pub_air_values_ready = false;
		pub_bme_values_ready = false;
	}
}
