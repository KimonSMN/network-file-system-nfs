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
    printf("%s\n", buffer);
}


bool handleCommand(FILE* fp, const char* input, int socketFd, const char* buffer, const char* source, const char* target) {
    if (checkCommand(input, "add")) {               // Command == add.
        if (source && target) {
            fprintf(fp, "[%s] Command add /%s -> /%s\n", getTime(), source, target); // might change it and tokenize dir,host,port
            fflush(fp);
            sendCommand(socketFd, buffer);
        } else
            printf("Usage: add <source> <target>"); 
    } else if (checkCommand(input, "cancel")) {     // Command == cancel.
        if (source) {
            fprintf(fp, "[%s] Command cancel /%s\n", getTime(), source);
            fflush(fp);
            sendCommand(socketFd, buffer);
        } else
            printf("Usage: cancel <source dir>");
    } else if (checkCommand(input, "shutdown")) {  // Command == shutdown.
        fprintf(fp, "[%s] Command shutdown\n", getTime());
        fflush(fp);
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
    char* array = malloc(MAX_BUFFER_LENGTH);
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

char* client_pull(const char* dir, const char* filename) {
    // construct the path
    char path[1024];
    getcwd(path, sizeof(path));
    strcat(path, "/");
    strcat(path, dir);
    strcat(path, "/");
    strcat(path, filename);

    // get file size
    char size[32];
    struct stat st;
    stat(path, &st);
    sprintf(size, "%ld ", st.st_size);
    int size_len = strlen(size);

    char* buffer = malloc(size_len + st.st_size + 1);

    snprintf(buffer, size_len + 1, "%ld ", st.st_size);

    int fd = open(path, O_RDONLY);

    ssize_t bytes_read = read(fd, buffer + size_len, st.st_size);
    buffer[size_len + bytes_read] = '\0';
    close(fd);
    return buffer;
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

