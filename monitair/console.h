#pragma once

#define CONSOLE_OK 0
#define CONSOLE_OFF 1

struct command {
	char * commandName;
	char * commandDescription;
	void(*actOnCommand)(char * commandLine);
};

void doHelp(char * commandLine);

void doDumpSettings(char * commandLine)
{
	PrintAllSettings();
}

void doStartWebServer(char * commandLine)
{
	struct process * serverProcess = startProcessByName("webserver");

	if (serverProcess != NULL)
	{
		serverProcess->getStatusMessage(serverProcess, processStatusBuffer, PROCESS_STATUS_BUFFER_SIZE);
		Serial.println(processStatusBuffer);
	}
	else
	{
		printf("Web server process not found");
	}
}

void doDumpStatus(char * commandLine)
{
	dumpSensorStatus();
	dumpProcessStatus();
	Serial.print("Heap: ");
	Serial.println(ESP.getFreeHeap());
}

void doRestart(char * commandLine)
{
	ESP.reset();
}

void doClear(char * commandLine)
{
	resetSettings();
	saveSettings();
	ESP.reset();
}

struct command userCommands [] = 
{
	{"help", "show all the commands", doHelp},
	{"host", "start the configuration web host", doStartWebServer},
	{"settings", "show all the setting values", doDumpSettings},
	{"status", "show the sensor status", doDumpStatus},
	{"restart", "restart the device", doRestart},
	{"clear", "clear all seeings and restart the device", doClear}
};

void doHelp(char * commandLine)
{
	Serial.printf("\n\nMonitair System Software Version %d.%d\n\nThese are all the available commands.\n\n", 
		settings.majorVersion, settings.minorVersion );

	int noOfCommands = sizeof(userCommands) / sizeof(struct command);

	for (int i = 0; i < noOfCommands; i++)
	{
		Serial.printf("    %s - %s\n", userCommands[i].commandName, userCommands[i].commandDescription);
	}

	Serial.printf("\nYou can view the value of any setting just by typing the setting name, for example:\n\n"
		"    devname\n\n"
		"- would show you the device name.\n"
		"You can assing a new value to a setting, for example:\n\n"
		"     devname=Rob\n\n"
		"- would set the name of the device to Rob.\n\n"
		"To see a list of all the setting names use the command dump.\n"
		"This displays all the settings, their values and names.\n"
	);
}

boolean findCommandName(command * com, char * name)
{
	int commandNameLength = strlen(com->commandName);


	for (int i = 0; i < commandNameLength; i++)
	{
		if (tolower(name[i] != com->commandName[i]))
			return false;
	}

	// reached the end of the name, character that follows should be zero (end of the string)
	// or a space delimiter to the next part of the command

	if (name[commandNameLength] == 0)
		return true;

	if (name[commandNameLength] == ' ')
		return true;

	return false;
}

struct command * findCommand(char * commandLine)
{
	int noOfCommands = sizeof(userCommands) / sizeof(struct command);

	for (int i = 0; i < noOfCommands; i++)
	{
		if (findCommandName(&userCommands[i], commandLine))
		{
			return &userCommands[i];
		}
	}
	return NULL;
}

boolean performCommand(char * commandLine)
{
	//Serial.print("Processing :");
	//Serial.println(commandLine);

	// Look for a command with that name

	command * comm = findCommand(commandLine);

	if (comm != NULL)
	{
		comm->actOnCommand(commandLine);
		return true;
	}

	// Look for a setting with that name

	processSettingCommandResult result;

	result = processSettingCommand(commandLine);

	switch (result)
	{
	case displayedOK:
		return true;
	case setOK:
		Serial.println("Value set successfully");
		return true;
	case settingNotFound:
		Serial.println("Command/setting not found");
		return false;
	case settingValueInvalid:
		Serial.println("Setting value invalid");
		return false;
	}

	return false;
}

void showHelp()
{
	doHelp("help");
}

#define SERIAL_BUFFER_SIZE 240
#define SERIAL_BUFFER_LIMIT SERIAL_BUFFER_SIZE-1

char serialReceiveBuffer[SERIAL_BUFFER_SIZE];

int serialReceiveBufferPos = 0;

void reset_serial_buffer()
{
	serialReceiveBufferPos = 0;
}

void actOnSerialCommand()
{
	performCommand(serialReceiveBuffer);
}

void bufferSerialChar(char ch)
{
	if (ch == '\n' || ch == '\r' || ch == 0)
	{
		if (serialReceiveBufferPos > 0)
		{
			serialReceiveBuffer[serialReceiveBufferPos] = 0;
			actOnSerialCommand();
			reset_serial_buffer();
		}
		return;
	}

	if (serialReceiveBufferPos < SERIAL_BUFFER_SIZE)
	{
		serialReceiveBuffer[serialReceiveBufferPos] = ch;
		serialReceiveBufferPos++;
	}
}

void checkSerialBuffer()
{
	while (Serial.available())
	{
		bufferSerialChar(Serial.read());
	}
}

int startConsole(struct process * consoleProcess)
{
	consoleProcess->status = CONSOLE_OK;
	return CONSOLE_OK;
}

int stopConsole(struct process * consoleProcess)
{
	consoleProcess->status = CONSOLE_OFF;
	return CONSOLE_OFF;
}

int updateConsole(struct process * consoleProcess)
{
	if (consoleProcess->status == CONSOLE_OK)
	{
		checkSerialBuffer();
	}
	return consoleProcess->status;
}

void consoleStatusMessage(struct process * consoleProcess, char * buffer, int bufferLength)
{
	switch (consoleProcess->status)
	{
	case CONSOLE_OK:
		snprintf(buffer, bufferLength, "Console OK");
		break;
	case CONSOLE_OFF:
		snprintf(buffer, bufferLength, "Console OFF");
		break;
	default:
		snprintf(buffer, bufferLength, "Console status invalid");
		break;
	}
}

void testConsoleCommands()
{
	if (performCommand("fred"))
		Serial.println("Command test failed - should not have found command fred");
	else
		Serial.println("Command fred not found - OK");

	if (performCommand("help"))
		Serial.println("Command help should have shown help");
	else
		Serial.println("Command help not found");
}
