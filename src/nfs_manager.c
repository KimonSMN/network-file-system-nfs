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
#define MAX_WORKERS 5


int main(int argc, char* argv[]) {

    char* manager_logfile = NULL;
    char* config_file = NULL;
    int worker_limit = 0;
    int port_number = 0;
    int bufferSize = 0;

    for (size_t i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-l") == 0) {
            manager_logfile = argv[++i];
        } else if (strcmp(argv[i], "-c") == 0) {
            config_file = argv[++i];
        }  else if (strcmp(argv[i], "-n") == 0) {
            worker_limit = atoi(argv[++i]);
        }  else if (strcmp(argv[i], "-p") == 0) {
            port_number = atoi(argv[++i]);
        }  else if (strcmp(argv[i], "-b") == 0) {
            bufferSize = atoi(argv[++i]);
        } else {
            printf("Parameter %s not recognized \n", argv[i]);
        }
    }

    if (manager_logfile == NULL || config_file == NULL || worker_limit < MAX_WORKERS || port_number < 0 || bufferSize < 0) {
        printf("Usage: ./nfs_manager -l <manager_logfile> -c <config_file> -n <worker_limit> -p <port_number> -b <bufferSize>\n");
        return 1;
    }

    int sockfd;
    int sendBytes;
    struct sockaddr_in servaddr;
    char sendLine[4096];

    char* source_address = "142.250.113.100";

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port_number);

    if (inet_pton(AF_INET, source_address, &servaddr.sin_addr) <= 0) 
        exit(1);

    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) 
        exit(1);

    

    return 0;
}