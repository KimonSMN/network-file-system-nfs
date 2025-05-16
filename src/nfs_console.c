#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>


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
    servaddr.sin_addr.s_addr = INADDR_ANY;

    inet_pton(AF_INET, host_ip, &servaddr.sin_addr);

    connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

    int active = 1;

    while (active) {
        printf("\n$ ");
        char write_buffer[1024] = {0};
        fgets(write_buffer, sizeof(write_buffer), stdin);
        write_buffer[strlen(write_buffer)-1] = '\0';

        write(sockfd, write_buffer, sizeof(write_buffer));

        char buffer[1024] = {0};
        read(sockfd, buffer, sizeof(buffer));
        printf("Response: %s\n", buffer);
    
    }

    close(sockfd);

    return EXIT_SUCCESS;

}