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
#include <stdarg.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>

bool checkCommand(const char* input, const char* command);

bool sendCommand(int socketFd, const char* buffer);

void printResponse(int socketFd);

bool handleCommand(FILE* fp, const char* input, int socketFd, const char* buffer, const char* source, const char* target);
char* getTime();

int check_dir(const char *path);

char* client_list(const char* source_dir);

void printf_fprintf(FILE* stream, char* format, ...);

int myconnect(const char* host, int port);

void client_pull(int client_fd, const char* dir, const char* filename);
#endif