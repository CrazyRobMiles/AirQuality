#pragma once

// from PixelControl.h
void updateReadingDisplay(float reading);

// from mqtt.h
void do_mqtt_send();

unsigned long mqtt_reading_retry_interval_in_millis;
unsigned long mqtt_reading_interval_in_millis;
unsigned long milliseconds_at_last_mqtt_update;

// return true if it is time for an update
bool mqtt_interval_has_expired()
{
	unsigned long time_in_millis = millis();
	unsigned long millis_since_last_mqtt_update = ulongDiff(time_in_millis, milliseconds_at_last_mqtt_update);


	if (millis_since_last_mqtt_update > mqtt_reading_interval_in_millis)
	{
		return true; // the interval 
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

// called when we have all the readings ready

void readings_ready()
{
	do_mqtt_send();
}


void setup_timing()
{
	mqtt_reading_interval_in_millis = settings.seconds_per_mqtt_update * 1000;
	milliseconds_at_last_mqtt_update = millis() ;
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
