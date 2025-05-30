#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "helper.h"

int main(int argc, char* argv[]){

    char*   console_logfile = NULL;
    char*   host_ip = NULL;
    int     host_port = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-l") == 0) {
            console_logfile = argv[++i];
        } else if (strcmp(argv[i], "-h") == 0) {
            host_ip = argv[++i];
        } else if (strcmp(argv[i], "-p") == 0) {
            host_port = atoi(argv[++i]);
        } else {
            printf("Parameter %s not recognized \n", argv[i]);
        }
    }
    if (console_logfile == NULL || host_ip == NULL || host_port < 0) {
        printf("Usage: ./nfs_console -l <console-logfile> -h <host_IP> -p <host_port>\n");
        exit(1);
    }

    // SOCKETS
    int sockfd;
    struct sockaddr_in servaddr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(host_port);
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    inet_pton(AF_INET, host_ip, &servaddr.sin_addr);
    connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

    FILE *consolelogFp = fopen(console_logfile, "w");
    if (consolelogFp == NULL) {
        return 1;
    }

    int active = 1;

    while (active) {
        printf("\n$ ");
        char write_buffer[1024] = {0};
        fgets(write_buffer, sizeof(write_buffer), stdin);
        write_buffer[strlen(write_buffer)-1] = '\0';

        char tmp[1024];
        strncpy(tmp, write_buffer, sizeof(write_buffer)); // safer.

        char* command = strtok(tmp, " ");   // token = add
        char* source = strtok(NULL, " ");   // token = source
        char* target = strtok(NULL, " ");   // token = target

        if(!handleCommand(consolelogFp, command, sockfd, write_buffer, source, target)) {
            active = 0;
        };
        // printResponse(sockfd);
    }
    fclose(consolelogFp);
    close(sockfd);

    return EXIT_SUCCESS;

}