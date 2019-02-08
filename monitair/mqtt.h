#pragma once

#include "processes.h"
#include "connectwifi.h"

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

#define MQTT_CONNECT_RETRY_INTERVAL_MSECS 5000

boolean publishReadingsToMQTT(char * buffer);

int startMQTT(struct process * mqttProcess);
int stopMQTT(struct process * mqttProcess);
int updateMQTT(struct process * mqttProcess);
void mqttStatusMessage(struct process * mqttProcess, char * buffer, int bufferLength);
