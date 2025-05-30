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

char* client_list(const char* source_dir){
    char path[1024];
    char* array = malloc(1024);
    array[0] = '\0';
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
            strcat(array, source_entity->d_name);
            strcat(array, "\n"); 
        }

        source_entity = readdir(source);
    }
    strcat(array, ".");

    closedir(source);
    return array;
}

void printf_fprintf(FILE* stream, char* format, ...){
    va_list ap;
    va_start(ap, format);
    vprintf(format, ap);
    va_end(ap);


    va_start(ap, format);
    vfprintf(stream, format, ap);
    va_end(ap);

}

int myconnect(const char* host, int port) {
    int socketfd;
    struct sockaddr_in servaddr;

    // Create socket
    socketfd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);

    inet_pton(AF_INET, host, &servaddr.sin_addr);
    if (connect(socketfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        close(socketfd);
        return -1;
    }
    return socketfd;
}

// char* get_files(char* buffer) {
//     char* files_string = malloc(sizeof(buffer));
//     files_string[0] = '\0';
    
    
//     char* file = strtok(buffer, "$");
//     while (file != NULL) {
//         strcat(files_string, file);
//         file = strtok(NULL ,"$");
//     }

//     return files_string;
// } 