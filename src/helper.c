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


bool handleCommand(FILE* fp, const char* input, int socketFd, const char* buffer, const char* source_dir, const char* source_host, const char* source_port, const char* target_dir, const char* target_host, const char* target_port) {
    if (checkCommand(input, "add")) {               // Command == add.
        if (source_dir && source_host && source_port && target_dir && target_host && target_port) {
            fprintf(fp, "[%s] Command add /%s@%s:%s -> /%s@%s:%s\n", getTime(), source_dir, source_host, source_port, target_dir, target_host, target_port); // might change it and tokenize dir,host,port
            fflush(fp);
            sendCommand(socketFd, buffer);
        } else
            printf("Usage: add <source@host:port> <target@host:port>\n"); 
    } else if (checkCommand(input, "cancel")) {     // Command == cancel.
        if (source_dir) {
            fprintf(fp, "[%s] Command cancel /%s\n", getTime(), source_dir);
            fflush(fp);
            sendCommand(socketFd, buffer);
        } else {
            printf("Usage: cancel <source_dir>\n");
            fflush(stdout);
        }
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

void printf_write(int socket_fd, char* format, ...){
    char* buffer = malloc(256);
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, 256, format, args);
    va_end(args);

    printf("%s", buffer);
    write(socket_fd, buffer, strlen(buffer));

    free(buffer);
}

void printf_fprintf_write(int socket_fd, FILE* stream, char* format, ...) {
    char* buffer = malloc(256);
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, 256, format, args);
    va_end(args);

    printf("%s", buffer);
    fprintf(stream, "%s", buffer);
    fflush(stream);

    write(socket_fd, buffer, strlen(buffer));

    free(buffer);
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

ssize_t safe_read(int socket_fd, char* buffer, size_t buffer_len){
    int safe_bytes_read = 0;
    char c;
    while (safe_bytes_read < buffer_len) {
        ssize_t bytes_read = read(socket_fd, &c, 1);
        if (bytes_read == 1) {
            buffer[safe_bytes_read++] = c;
            if (c == '\n') {
                break;
            }
        } else if (bytes_read == 0) {
            break; // EOF
        }
    }
    buffer[safe_bytes_read] = '\0';
    return safe_bytes_read;
}

//            char command[512];
// snprintf(command, sizeof(command), "LIST %s\n", source_dir);
// write(client_socket, command, sizeof(command));
int write_list(int socket_fd, const char* source_dir){
    char* buffer = malloc(1024); 
    if (buffer == NULL) 
        return EXIT_FAILURE;

    snprintf(buffer, 1024, "LIST %s\n", source_dir);
    write(socket_fd, buffer, strlen(buffer));

    free(buffer);
    return EXIT_SUCCESS;
}

int write_pull(int socket_fd, const char* source_dir, const char* filename){
    char* buffer = malloc(1024); 
    if (buffer == NULL) 
        return EXIT_FAILURE;

    snprintf(buffer, 1024, "PULL %s/%s\n", source_dir, filename);
    write(socket_fd, buffer, strlen(buffer));

    free(buffer);
    return EXIT_SUCCESS;
}

