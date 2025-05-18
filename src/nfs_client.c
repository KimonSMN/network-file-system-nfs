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

    if (strcmp(argv[1], "-p") == 0 && argc == 3) {  // argc == 3 to avoid seg fault if only flag was provided without port_number.
        port_number = atoi(argv[2]);
    } else {
        printf("Problem with argument %s.\n", argv[1]);
    } 
    if (port_number <= 0) {
        printf("Usage: ./nfs_client -p <port_number>\n");
        return 1;
    }

    return EXIT_SUCCESS;
}