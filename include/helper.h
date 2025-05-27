#ifndef HELPER
#define HELPER

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>

bool checkCommand(const char* input, const char* command);

bool sendCommand(int socketFd, const char* buffer);

void printResponse(int socketFd);

bool handleCommand(const char* input, int socketFd, const char* buffer, const char* source, const char* target);

char* getTime();

int check_dir(const char *path);

char* client_list(const char* source_dir);

#endif