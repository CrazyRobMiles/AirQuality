#pragma once

#define MQTT_DISPLAY_TIMEOUT 3000

unsigned long mqtt_timer_start;


Client * espClient;

PubSubClient * mqttPubSubClient;

#define MQTT_RECEIVE_BUFFER_SIZE 240
char mqtt_receive_buffer[MQTT_RECEIVE_BUFFER_SIZE];

#define MQTT_SEND_BUFFER_SIZE 240

char mqtt_send_buffer[MQTT_SEND_BUFFER_SIZE];

boolean first_mqtt_message = true;

boolean send_buffer_to_mqtt(char * buffer)
{
	if (mqttState != ConnectedToMQTTServer)
	{
		TRACELN("MQTT not connected yet");
		return false;
	}

	if (first_mqtt_message)
	{
		Serial.println("Sending first MQTT message");
		Serial.println(buffer);
		first_mqtt_message = false;
	}

	TRACELN(mqtt_send_buffer);

	mqttPubSubClient->publish(settings.mqttPublishTopic, buffer);
	TRACELN("MQTT sent");
	return true;
}

boolean send_to_mqtt()
{
	if (pub_got_gps_fix)
	{
		// got a GPS fix - how old is it?
		ulong fix_age = ulongDiff(millis(), pub_ticks_at_last_gps_update);
		if (fix_age < MILLIS_LIFETIME_OF_GPS_FIX)
		{
			double latitude = pub_latitude_mdeg / 1000000.0;
			double longitude = pub_longitude_mdeg / 1000000.0;

			// still regard the fix as valid - add lat and long to the message
			//sprintf(mqtt_send_buffer,
			//	"{\"dev\" : \"%s\", \"PM10\" : %.2f, \"PM25\" : %.2f, \"Lat\" : %.6f, \"Long\" : %.6f, \"timestamp\" : \"%s %s %d %d %02d:%02d:%02d GMT+0000\"}",
			//	settings.mqttName,
			//	pub_ppm_10,
			//	pub_ppm_25,
			//	latitude,
			//	longitude,
			//	dayNames[pub_day_of_week],
			//	monthNames[pub_month],
			//	pub_day,
			//	pub_year,
			//	pub_hour,
			//	pub_minute,
			//	pub_second);
			sprintf(mqtt_send_buffer,
				"{\"dev\":\"%s\", \"temp\":%.2f, \"humidity\" : %.2f, \"pressure\" : %.2f, \"PM10\" : %.2f, \"PM25\" : %.2f, \"Lat\" : %.6f, \"Long\" : %.6f, \"timestamp\" : \"%s %s %d %d %02d:%02d:%02d GMT+0000\"}",
				settings.mqttName,
				pub_temp,
				pub_humidity,
				pub_pressure,
				pub_ppm_10,
				pub_ppm_25,
				latitude,
				longitude,
				dayNames[pub_day_of_week],
				monthNames[pub_month],
				pub_day,
				pub_year,
				pub_hour,
				pub_minute,
				pub_second);
			return send_buffer_to_mqtt(mqtt_send_buffer);
		}
	}

	// otherwise send the message with no location information

	//sprintf(mqtt_send_buffer,
	//	"{\"dev\" : \"%s\", \"PM25\" : %.2f, \"timestamp\" : \"%s %s %d %d %02d:%02d:%02d GMT+0000\"}",
	//	settings.mqttName,
	//	pub_ppm_25,
	//	dayNames[pub_day_of_week],
	//	monthNames[pub_month],
	//	pub_day,
	//	pub_year,
	//	pub_hour,
	//	pub_minute,
	//	pub_second);

	sprintf(mqtt_send_buffer,
		"{\"dev\":\"%s\",\"temp\":%.2f, \"humidity\" : %.2f, \"pressure\" : %.2f, \"PM10\" : %.2f, \"PM25\" : %.2f, \"timestamp\" : \"%s %s %d %d %02d:%02d:%02d GMT+0000\"}",
		settings.mqttName,
		pub_temp,
		pub_humidity,
		pub_pressure,
		pub_ppm_10,
		pub_ppm_25,
		dayNames[pub_day_of_week],
		monthNames[pub_month],
		pub_day,
		pub_year,
		pub_hour,
		pub_minute,
		pub_second);

	return send_buffer_to_mqtt(mqtt_send_buffer);
}

void mqtt_deliver_command_result(char * result)
{
	send_buffer_to_mqtt(result);
}

void callback(char* topic, byte* payload, unsigned int length)
{
	TRACE("Message arrived in topic: ");
	TRACELN(topic);

	TRACE("Message:");
	int i;
	for (i = 0; i < length; i++) {
		TRACE((char)payload[i]);
		mqtt_receive_buffer[i] = (char)payload[i];
	}

	// Put the terminator on the string

	mqtt_receive_buffer[i] = 0;

	TRACELN();
	TRACELN("-----------------------");

	act_onJson_command(mqtt_receive_buffer, mqtt_deliver_command_result);
}

void send_to_mqtt_with_retry()
{
	if (send_to_mqtt())
	{
		mqtt_reading_interval_in_millis = settings.mqttSecsPerUpdate * 1000;
	}
	else
	{
		mqtt_reading_interval_in_millis = settings.seconds_per_mqtt_retry * 1000;
	}
}

void do_mqtt_send()
{
	if (settings.mqtt_enabled)
	{
		unsigned long time_in_millis = millis();
		unsigned long millis_since_last_mqtt_update = ulongDiff(time_in_millis, milliseconds_at_last_mqtt_update);

		if (millis_since_last_mqtt_update > mqtt_reading_interval_in_millis)
		{
			// handle any lag to make sure that timings don't drift
			unsigned long time_to_update = millis_since_last_mqtt_update - mqtt_reading_interval_in_millis;
			// set the time of the update to allow for any delay
			milliseconds_at_last_mqtt_update = time_in_millis - time_to_update;
			send_to_mqtt_with_retry();
		}
	}
	else
	{
		Serial.println("MQTT not sent - MQTT turned off");
	}

	if (pub_mqtt_force_send)
	{
		if (send_to_mqtt())
		{
			pub_mqtt_force_send = false;
		}
	}
}

void setup_mqtt()
{
	if (settings.mqttSecureSockets)
	{
		espClient = new WiFiClientSecure();
	}
	else
	{
		espClient = new WiFiClient();
	}

	mqttPubSubClient = new PubSubClient(*espClient);

	pub_mqtt_force_send = false;
	mqttState = AwaitingWiFi;
}

void mqtt_connect_failed()
{
	TRACE("MQTT failed with state ");
	TRACELN(mqttPubSubClient->state());
	mqtt_timer_start = millis();
	update_action(settings.mqttName, "Connect Failed");
	mqttState = ShowingConnectToMQTTServerFailed;
}

void mqtt_connected()
{
	TRACELN("MQTT connected");
	mqtt_timer_start = millis();
	update_action(settings.mqttName, "Connected OK");
	mqttPubSubClient->subscribe(settings.mqttSubscribeTopic);
	mqttState = ShowingConnectedToMQTTServer;
	pub_mqtt_force_send = true;
}

void attempt_mqtt_connect()
{
	TRACELN(settings.mqttName);
	TRACELN(settings.mqttUser);
	TRACELN(settings.mqttPassword);
	TRACELN(settings.mqttPort);
	TRACELN(settings.mqttServer);

	if (mqttPubSubClient->connect(settings.mqttName, settings.mqttUser, settings.mqttPassword))
	{
		mqtt_connected();
	}
	else
	{
		mqtt_connect_failed();
	}
}

void loop_mqtt()
{
	unsigned long elapsed_time;

	switch (mqttState)
	{
	case AwaitingWiFi:

		if (wifiState == WiFiConnected)
		{
			// Start the MQTT connection running
			start_action(settings.mqttName, "Connecting to MQTT");
			mqttPubSubClient->setServer(settings.mqttServer, settings.mqttPort);
			mqttPubSubClient->setCallback(callback);
			attempt_mqtt_connect();
		}
		break;

	case ConnectingToMQTTServer:
		attempt_mqtt_connect();
		break;

	case ShowingConnectedToMQTTServer:
		elapsed_time = millis() - mqtt_timer_start;

		if (elapsed_time > MQTT_DISPLAY_TIMEOUT)
		{
			end_action();
			mqttState = ConnectedToMQTTServer;
		}
		mqttPubSubClient->loop();
		break;

	case ShowingConnectToMQTTServerFailed:
		elapsed_time = millis() - mqtt_timer_start;

		if (elapsed_time > MQTT_DISPLAY_TIMEOUT)
		{
			end_action();
			mqttState = ConnectToMQTTServerFailed;
		}
		break;

	case ConnectedToMQTTServer:
		mqttPubSubClient->loop();
		break;

	case ConnectToMQTTServerFailed:
		break;
	}
}

