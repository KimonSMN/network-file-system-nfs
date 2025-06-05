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
#include <fcntl.h>

#include "helper.h"

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
    int client_fd;
    while (1) {
        client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        char header[4096] = {0};

        safe_read(client_fd, header, sizeof(header));
        printf("[+]Recived Header: %s\n", header);
        printf("[+]End of receiving...\n");

        char *cmd = strtok(header, " \n");
        if (strcmp(cmd, "LIST") == 0) {
            char *source_dir = strtok(NULL, "\n");

            char* arr = client_list(source_dir);
            if (arr != NULL) {
                write(client_fd, arr, strlen(arr));
                free(arr);            
            }
        } else if (strcmp(cmd, "PULL") == 0) {
            char *source_dir = strtok(NULL, " /");
            char *source_file = strtok(NULL, " \n");
            char* buffer = client_pull(source_dir, source_file);
            if (buffer != NULL) {
                printf("[+]Buffer:\n\t %s\n", buffer);
                write(client_fd, buffer, strlen(buffer));
                free(buffer);     
                shutdown(client_fd, SHUT_WR);
            }
        } else if (strcmp(cmd, "PUSH") == 0) {
            char* target_dir = strtok(NULL, " /");
            char* target_file = strtok(NULL, " \n");

            char path[256];
            snprintf(path, sizeof(path), "%s/%s", target_dir, target_file);
            printf("[+]Constructed path: %s\n", path);

            int target_fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0777);

            char chunk_string[32];
            char data_buf[4096]; 
            int chunk_size;

            while (1) {
                ssize_t bytes_read = safe_read(client_fd, chunk_string, sizeof(chunk_string));
                if (bytes_read <= 0) 
                    break;  // EOF

                chunk_size = atoi(chunk_string); 

                int counter = 0;
                while (counter < chunk_size) {
                    ssize_t num_read = read(client_fd, data_buf + counter, chunk_size - counter);
                    if (num_read <= 0) // EOF
                        break;
                    counter += num_read;
                }

                char* pointer_to_space = strchr(data_buf, ' ') + 1; // returns a pointer to first occurance of ' ' (after filesize that is). so "9 testfile , pointer_to space is at " testfile"

                size_t data_len = counter - (pointer_to_space - data_buf);
                write(target_fd, pointer_to_space, data_len);
                
                printf("[+]Wrote %d bytes to %s\n", counter, target_file);
            }
            close(target_fd);
        } else {
            printf("ERROR: Unknown command\n");
        }
    }
    close(client_fd);
    close(server_fd);

    return EXIT_SUCCESS;
}