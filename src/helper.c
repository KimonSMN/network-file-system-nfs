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

int check_dir(const char *path) {
    struct stat st;
    if(stat(path, &st) != 0) 
        return 1;
    
    return 0;
}

char** client_list(const char* source_dir){
    char path[1024];
    char *array[] = {0}; // pointer array to strings
    int counter = 0;
    getcwd(path, sizeof(path));
    strcat(path, source_dir);

    DIR* source = opendir(path);
    if (!source) {
        return NULL;
    }
    struct dirent* source_entity;
    source_entity = readdir(source);

    while (source_entity != NULL) {      
        if(strcmp(source_entity->d_name, ".") != 0 && strcmp(source_entity->d_name, "..") != 0 ) {
            printf("%s\n", source_entity->d_name);
            array[counter] = source_entity->d_name;
            printf("array %d : %s\n", counter, array[counter]);
            counter++;
        }

        source_entity = readdir(source);
    }
    array[counter] = ".";
    printf("array %d : %s\n", counter, array[counter]);

    return array;
}