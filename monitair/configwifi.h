#pragma once

#include "processes.h"

int startWifiConfig(struct process * wifiConfigProcess);
int updateWifiConfig(struct process * wifiConfigProcess);
int stopWifiConfig(struct process * wifiConfigProcess);
void wifiConfigStatusMessage(struct process * wifiConfigProcess, char * buffer, int bufferLength);
