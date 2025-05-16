#include <stdio.h>
#include <string.h>
#include <stdlib.h>


int main(int argc, char* argv[]) {

    int port_number = 0;

    if (strcmp(argv[1], "-p") == 0) {
        port_number = argv[2];
    } else {
        printf("Parameter %s not recognized \n", argv[1]);
    } 

    if (port_number < 0) {
        printf("Usage: ./nfs_client -p <port_number>\n");
        return 1;
    }


    return EXIT_SUCCESS;
}