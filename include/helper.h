#ifndef HELPER
#define HELPER

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>

bool checkCommand(const char* input, const char* command);

bool sendCommand(int socketFd, const char* buffer);

void printResponse(int socketFd);

void handleCommand(const char* input, int socketFd, const char* buffer, const char* source, const char* target);

#endif