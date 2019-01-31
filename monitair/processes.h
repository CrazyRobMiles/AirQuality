#pragma once

#define STATUS_DESCRIPTION_LENGTH 200
#define PROCESS_OK 0

struct process
{
	char * processName;
	int(*startProcess)(struct process *);
	int(*udpateProcess)(struct process *);
	int(*stopProcess)(struct process *);
	void(*getStatusMessage)(struct process *, char * buffer, int bufferLength);
	boolean activeAtStart;
	boolean beingUpdated;  // true if the process is running
	boolean compatibleWithSerial; // true if this process can be updated when the serial port is active
	int status;      // zero means OK - any other value is an error state
	unsigned long activeTime;
};

int startWifi(struct process * wifiProcess);
int stopWiFi(struct process * wifiProcess);
int updateWifi(struct process * wifiProcess);
void wifiStatusMessage(struct process * wifiProcess, char * buffer, int bufferLength);

int startMQTT(struct process * mqttProcess);
int stopMQTT(struct process * mqttProcess);
int updateMQTT(struct process * mqttProcess);
void mqttStatusMessage(struct process * mqttProcess, char * buffer, int bufferLength);

int startConsole(struct process * consoleProcess);
int stopConsole(struct process * consoleProcess);
int updateConsole(struct process * consoleProcess);
void consoleStatusMessage(struct process * consoleProcess, char * buffer, int bufferLength);

int startWebServer(struct process * webserverProcess);
int updateWebServer(struct process * webserverProcess);
int stopWebserver(struct process * webserverProcess);
void webserverStatusMessage(struct process * webserverProcess, char * buffer, int bufferLength);

int startOtaUpdate(struct process * otaUpdateProcess);
int updateOtaUpdate(struct process * otaUpdateProcess);
int stopOtaUpdate(struct process * otaUpdateProcess);
void otaUpdateStatusMessage(struct process * otaUpdateProcess, char * buffer, int bufferLength);

int startPixel(struct process * pixelProcess);
int updatePixel(struct process * pixelProcess);
int stopPixel(struct process * pixelProcess);
void pixelStatusMessage(struct process * pixelProcess, char * buffer, int bufferLength);

int startInputSwitch(struct process * inputSwitchProcess);
int updateInputSwitch(struct process * inputSwitchProcess);
int stopInputSwitch(struct process * inputSwitchProcess);
void inputSwitchStatusMessage(struct process * inputSwitchProcess, char * buffer, int bufferLength);

int startWifiConfig(struct process * wifiConfigProcess);
int updateWifiConfig(struct process * wifiConfigProcess);
int stopWifiConfig(struct process * wifiConfigProcess);
void wifiConfigStatusMessage(struct process * wifiConfigProcess, char * buffer, int bufferLength);

struct process PixelProcess = { "Pixel", startPixel, updatePixel, stopPixel, pixelStatusMessage, true, false, true, 0, 0 };
struct process WiFiProcessDescriptor = { "WiFi", startWifi, updateWifi, stopWiFi, wifiStatusMessage, true, false, false, 0, 0 };
struct process MQTTProcessDescriptor = { "MQTT", startMQTT, updateMQTT, stopMQTT, mqttStatusMessage, true,  false, false, 0, 0 };
struct process ConsoleProcessDescriptor = { "Console", startConsole, updateConsole, stopConsole, consoleStatusMessage, true, false, false, 0, 0 };
struct process WebServerProcessDescriptor = { "Webserver", startWebServer, updateWebServer, stopWebserver, webserverStatusMessage, false, false, false, 0, 0 }; // don't start the web server by default
struct process OTAUpdateProcess = { "OTA", startOtaUpdate, updateOtaUpdate, stopOtaUpdate, otaUpdateStatusMessage, false, false, false, 0, 0 }; // don't start the ota update by default
struct process InputSwitchProcess = { "Input switch", startInputSwitch, updateInputSwitch, stopInputSwitch, inputSwitchStatusMessage, true, false, true, 0, 0 }; 

struct process WiFiConfigProcess = { "Wifi Config", startWifiConfig, updateWifiConfig, stopWifiConfig, wifiConfigStatusMessage, true, false, true, 0, 0 }; 

struct process * runningProcessList[] =
{
	&PixelProcess,
	&WiFiProcessDescriptor,
	&ConsoleProcessDescriptor,
	&WebServerProcessDescriptor,
	&MQTTProcessDescriptor,
	&OTAUpdateProcess,
	&InputSwitchProcess
};

struct process * wifiConfigProcessList[] =
{
	&WiFiConfigProcess,
	&WebServerProcessDescriptor
};



struct process * findProcessByName(char * name)
{
	for (int i = 0; i < sizeof(runningProcessList)/sizeof(struct process *); i++)
	{
		if (strcasecmp(runningProcessList[i]->processName, name) == 0)
		{
			return runningProcessList[i];
		}
	}
	return NULL;
}

struct process * startProcessByName(char * name)
{
	struct process * targetProcess = findProcessByName(name);

	if (targetProcess != NULL)
	{
		targetProcess->startProcess(targetProcess);
		targetProcess->beingUpdated = true;
	}

	return targetProcess;
}

#define PROCESS_STATUS_BUFFER_SIZE 300

char processStatusBuffer[PROCESS_STATUS_BUFFER_SIZE];

void startProcess(process * proc)
{
	Serial.printf("Starting process %s: ", proc->processName);
	proc->startProcess(proc);
	proc->getStatusMessage(proc, processStatusBuffer, PROCESS_STATUS_BUFFER_SIZE);
	Serial.printf("%s\n", processStatusBuffer);
	proc->beingUpdated = true;  // process only gets updated after it has started
	addStatusItem(proc->status == PROCESS_OK);
}

void startDeviceProcesses()
{
	// start all the management processes
	for (int i = 0; i < sizeof(runningProcessList) / sizeof(struct process *); i++)
	{
		process * proc = runningProcessList[i];
		if (!proc->activeAtStart) continue;
		startProcess(proc);
	}
}

void startWifiConfigProcesses()
{
	for (int i = 0; i < sizeof(wifiConfigProcessList) / sizeof(struct process *); i++)
	{
		process * proc = wifiConfigProcessList[i];
		startProcess(proc);
	}
}

void updateWifiConfigProcesses()
{
	for (int i = 0; i < sizeof(wifiConfigProcessList) / sizeof(struct process *); i++)
	{
		process * proc = wifiConfigProcessList[i];
		proc->udpateProcess(proc);
	}
}

void dumpProcessStatus()
{
	Serial.println("Processes");

	for (int i = 0; i < sizeof(runningProcessList) / sizeof(struct process *); i++)
	{
		runningProcessList[i]->getStatusMessage(runningProcessList[i], processStatusBuffer, PROCESS_STATUS_BUFFER_SIZE);
		Serial.printf("    %s Active time(microsecs):", processStatusBuffer);
		Serial.println(runningProcessList[i]->activeTime);
	}
}

void updateProcess(struct process * process)
{
	unsigned long startMicros = micros();
	process->udpateProcess(process);
	process->activeTime = ulongDiff(micros(), startMicros);
}

void updateProcesses()
{
	for (int i = 0; i < sizeof(runningProcessList) / sizeof(struct process *); i++)
	{
		if (runningProcessList[i]->beingUpdated)
		{
			updateProcess(runningProcessList[i]);
		}
	}
}

void updateSerialCompatibleProcesses()
{
	for (int i = 0; i < sizeof(runningProcessList) / sizeof(struct process *); i++)
	{
		if (runningProcessList[i]->compatibleWithSerial &&
			runningProcessList[i]->beingUpdated)
		{
			updateProcess(runningProcessList[i]);
		}
	}
}

void activeDelay(long delayTime)
{
	for (int i = 0; i < delayTime; i++)
	{
		updateProcesses();
		delay(1);
	}
}

void activeSerialCompatibleDelay(long delayTime)
{
	for (int i = 0; i < delayTime; i++)
	{
		updateSerialCompatibleProcesses();
		delay(1);
	}
}

void displayProcessStatus()
{
	for (int i = 0; i < sizeof(runningProcessList) / sizeof(struct process *); i++)
	{
		process * displayProcess = runningProcessList[i];
		addStatusItem(displayProcess->status == PROCESS_OK);
	}
}

