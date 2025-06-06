#ifndef HELPER
#define HELPER

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_BUFFER_LENGTH 4096

bool checkCommand(const char* input, const char* command);

bool sendCommand(int socketFd, const char* buffer);

void printResponse(int socketFd);

bool handleCommand(FILE* fp, const char* input, int socketFd, const char* buffer, const char* source_dir, const char* source_host, const char* source_port, const char* target_dir, const char* target_host, const char* target_port);

char* getTime();

int check_dir(const char *path);

char* client_list(const char* source_dir);

void printf_fprintf(FILE* stream, char* format, ...);

void printf_write(int socket_fd, char* format, ...);

void printf_fprintf_write(int socket_fd, FILE* stream, char* format, ...);

int myconnect(const char* host, int port);

char* client_pull(const char* dir, const char* filename);

ssize_t safe_read(int socket_fd, char* buffer, size_t buffer_len);

int write_list(int socket_fd, const char* source_dir);
int write_pull(int socket_fd, const char* source_dir, const char* filename);
#endif