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

int main(int argc, char* argv[]) {

    int port_number = 0;

    if (argc == 3 && strcmp(argv[1], "-p") == 0) {  // argc == 3 to avoid seg fault if only flag was provided without port_number.
        port_number = atoi(argv[2]);
    } else {
        printf("Problem with argument %s.\n", argv[1]);
    } 
    if (port_number <= 0) {
        printf("Usage: ./nfs_client -p <port_number>\n");
        return 1;
    }

    // Setup server socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port_number);
    
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
    listen(server_fd, 5);
    printf("nfs_client listening on port %d...\n", port_number);

    while (1) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        char buffer[1024] = {0};
        read(client_fd, buffer, sizeof(buffer));

        // Parse command
        char *cmd = strtok(buffer, " \n");
        if (strcmp(cmd, "LIST") == 0) {
            char *dir = strtok(NULL, "\n");
            printf("YOOOOOOOOO WORKED %s\n", dir);
        }else {
            dprintf(client_fd, "ERROR: Unknown command\n");
        }

        close(client_fd);

    }
    close(server_fd);

    return EXIT_SUCCESS;
}