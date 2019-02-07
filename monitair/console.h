#pragma once

#include "processes.h"

#define CONSOLE_OK 0
#define CONSOLE_OFF 1

int startConsole(struct process * consoleProcess);
int stopConsole(struct process * consoleProcess);
int updateConsole(struct process * consoleProcess);
void consoleStatusMessage(struct process * consoleProcess, char * buffer, int bufferLength);

