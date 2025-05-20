#include "helper.h"

bool checkCommand(const char* input, const char* command){

    if (input == NULL || command == NULL)
        return false;

    if (strcmp(input, command) == 0) 
        return true;
    
    return false;
}

bool sendCommand(int socketFd, const char* buffer){
    ssize_t numWritten = write(socketFd, buffer, strlen(buffer));
    if (numWritten <= 0) {
        return false;
    }
    printResponse(socketFd);
    return true;
}

void printResponse(int socketFd){
    char buffer[1024] = {0};
    read(socketFd, buffer, sizeof(buffer));
    printf("Response: %s\n", buffer);
}


bool handleCommand(const char* input, int socketFd, const char* buffer, const char* source, const char* target) {
    if (checkCommand(input, "add")) {               // Command == add.
        if (source && target) {
            sendCommand(socketFd, buffer);
        } else
            printf("Usage: add <source> <target>"); 
    } else if (checkCommand(input, "cancel")) {     // Command == cancel.
        if (source) {
            sendCommand(socketFd, buffer);
        } else
            printf("Usage: cancel <source dir>");
    } else if (checkCommand(input, "shutdown")) {  // Command == shutdown.
        sendCommand(socketFd, buffer);
        return false;
    } else
        printf("Command not recognized.\n");       // Unrecognized Command.

    return true;
}

char* getTime(){
    static char current_time[32];
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    strftime(current_time, sizeof(current_time), "%Y-%m-%d %H:%M:%S", &tm);
    return current_time;
}