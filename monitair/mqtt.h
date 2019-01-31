#pragma once

#define MQTT_OK 0
#define MQTT_OFF 1
#define MQTT_ERROR_NO_WIFI -1
#define MQTT_ERROR_BAD_PROTOCOL -2
#define MQTT_ERROR_BAD_CLIENT_ID -3
#define MQTT_ERROR_CONNECT_UNAVAILABLE -4
#define MQTT_ERROR_BAD_CREDENTIALS -5
#define MQTT_ERROR_CONNECT_UNAUTHORIZED -6
#define MQTT_ERROR_CONNECT_FAILED -7
#define MQTT_ERROR_CONNECT_ERROR -8
#define MQTT_ERROR_CONNECT_MESSAGE_FAILED -9
#define MQTT_ERROR_LOOP_FAILED -10

unsigned long mqtt_timer_start;

Client * espClient = NULL;

PubSubClient * mqttPubSubClient = NULL;

#define MQTT_RECEIVE_BUFFER_SIZE 240
char mqtt_receive_buffer[MQTT_RECEIVE_BUFFER_SIZE];

#define MQTT_SEND_BUFFER_SIZE 240

char mqtt_send_buffer[MQTT_SEND_BUFFER_SIZE];

boolean first_mqtt_message = true;

void callback(char* topic, byte* payload, unsigned int length)
{
	int i;
	for (i = 0; i < length; i++) {
		mqtt_receive_buffer[i] = (char)payload[i];
	}

	// Put the terminator on the string
	mqtt_receive_buffer[i] = 0;
	// Now do something with the string....
	Serial.printf("Received from MQTT: %s\n", mqtt_receive_buffer);
}

int mqttConnectErrorNumber;

int startMQTT(struct process * mqttProcess)
{
	if (WiFiProcessDescriptor.status != WIFI_OK)
	{
		mqttProcess->status = MQTT_ERROR_NO_WIFI;
		return MQTT_ERROR_NO_WIFI;
	}

	if (espClient == NULL)
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

		mqttPubSubClient->setServer(settings.mqttServer, settings.mqttPort);
		mqttPubSubClient->setCallback(callback);
	}

	if (!mqttPubSubClient->connect(settings.deviceName, settings.mqttUser, settings.mqttPassword))
	{
		switch (mqttPubSubClient->state())
		{
		case MQTT_CONNECT_BAD_PROTOCOL:
			mqttProcess->status = MQTT_ERROR_BAD_PROTOCOL;
			break;
		case MQTT_CONNECT_BAD_CLIENT_ID:
			mqttProcess->status = MQTT_ERROR_BAD_CLIENT_ID;
			break;
		case MQTT_CONNECT_UNAVAILABLE:
			mqttProcess->status = MQTT_ERROR_CONNECT_UNAVAILABLE;
			break;
		case MQTT_CONNECT_BAD_CREDENTIALS:
			mqttProcess->status = MQTT_ERROR_BAD_CREDENTIALS;
			break;
		case MQTT_CONNECT_UNAUTHORIZED:
			mqttProcess->status = MQTT_ERROR_CONNECT_UNAUTHORIZED;
			break;
		case MQTT_CONNECTION_TIMEOUT:
			mqttProcess->status = MQTT_ERROR_BAD_PROTOCOL;
			break;
		case MQTT_CONNECT_FAILED:
			mqttProcess->status = MQTT_ERROR_CONNECT_FAILED;
			break;
		default:
			mqttConnectErrorNumber = mqttPubSubClient->state();
			mqttProcess->status = MQTT_ERROR_CONNECT_ERROR;
			break;
		}
		return mqttProcess->status;
	}

	mqttPubSubClient->subscribe(settings.mqttSubscribeTopic);

	snprintf(mqtt_send_buffer, MQTT_SEND_BUFFER_SIZE,
		"{\"dev\":\"%s\", \"status\":\"starting\"}",
		settings.deviceName);

	if (!mqttPubSubClient->publish(settings.mqttReportTopic, mqtt_send_buffer))
	{
		Serial.println("publish failed");
		mqttProcess->status = MQTT_ERROR_CONNECT_MESSAGE_FAILED;
		return MQTT_ERROR_CONNECT_MESSAGE_FAILED;
	}

	mqttProcess->status = MQTT_OK;
	return MQTT_OK;
}

boolean publishReadingsToMQTT(char * buffer)
{
	if(MQTTProcessDescriptor.status == MQTT_OK)
		return mqttPubSubClient->publish(settings.mqttPublishTopic, buffer);

	return false;
}

int stopMQTT(struct process * mqttProcess)
{
	mqttProcess->status = MQTT_OFF;
	return MQTT_OFF;
}

int updateMQTT(struct process * mqttProcess)
{
	if (mqttProcess->status == MQTT_OK)
	{
		if (!mqttPubSubClient->loop())
		{
			mqttProcess->status = MQTT_ERROR_LOOP_FAILED;
		}
	}
	else
	{
		Serial.println("Restarting MQTT");
		mqttProcess->status = startMQTT(mqttProcess);
	}
	return mqttProcess->status;
}

void mqttStatusMessage(struct process * mqttProcess, char * buffer, int bufferLength)
{
	switch (mqttProcess->status)
	{
	case MQTT_OK:
		snprintf(buffer, bufferLength, "MQTT OK");
		break;
	case MQTT_OFF:
		snprintf(buffer, bufferLength, "MQTT OFF");
		break;
	case MQTT_ERROR_NO_WIFI:
		snprintf(buffer, bufferLength, "MQTT error no WiFi");
		break;
	case MQTT_ERROR_BAD_PROTOCOL:
		snprintf(buffer, bufferLength, "MQTT error bad protocol");
		break;
	case MQTT_ERROR_BAD_CLIENT_ID:
		snprintf(buffer, bufferLength, "MQTT error bad client ID");
		break;
	case MQTT_ERROR_CONNECT_UNAVAILABLE:
		snprintf(buffer, bufferLength, "MQTT error connect unavailable");
		break;
	case MQTT_ERROR_BAD_CREDENTIALS:
		snprintf(buffer, bufferLength, "MQTT error bad credentials");
		break;
	case MQTT_ERROR_CONNECT_UNAUTHORIZED:
		snprintf(buffer, bufferLength, "MQTT error connect unauthorized");
		break;
	case MQTT_ERROR_CONNECT_FAILED:
		snprintf(buffer, bufferLength, "MQTT error connect failed");
		break;
	case MQTT_ERROR_CONNECT_ERROR:
		snprintf(buffer, bufferLength, "MQTT error connect error %d", mqttConnectErrorNumber);
		break;
	case MQTT_ERROR_CONNECT_MESSAGE_FAILED:
		snprintf(buffer, bufferLength, "MQTT error connect message failed");
		break;
	case MQTT_ERROR_LOOP_FAILED:
		snprintf(buffer, bufferLength, "MQTT error loop failed");
		break;
	default:
		snprintf(buffer, bufferLength, "MQTT failed but I'm not sure why");
		break;
	}
}

