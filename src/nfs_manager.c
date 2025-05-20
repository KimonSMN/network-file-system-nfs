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

#include "sync_info_mem_store.h"
#include "queue.h"
#include "helper.h"

#define MAX_WORKERS 5


void* foo(void* arg) {
    
    // Get current thread ID
    pthread_t thisThread = pthread_self();
    printf("Current thread ID: %lu\n",(unsigned long)thisThread);
    return NULL;
}

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


    // Δηµιουργεί ένα socket στο port που δόθηκε.

    // int serverfd, clientfd;
    // struct sockaddr_in servaddr;

    // if ((serverfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    //     perror("socket");
    //     exit(1);
    // }

    // memset(&servaddr, 0, sizeof(servaddr));
    // servaddr.sin_family = AF_INET;
    // servaddr.sin_addr.s_addr = INADDR_ANY;
    // servaddr.sin_port = htons(port_number);

    // bind(serverfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    // listen(serverfd, 5);

    // char buffer[1024];

    // clientfd = accept(serverfd, NULL, NULL);


    // Ακολούθως, ετοιµάζει τις διάφορες δοµές δεδοµένων που θα χρειαστεί για το συγχρονισµό των καταλόγων.
    hashTable* table = init_hash_table();   // Initialize the hash-table.
    queue* q = init_queue();                // Initialize the queue.
    
    // Ετοιµάζει και δηµιουργεί ένα worker thread pool.

    // pthread_t worker_thread_pool[worker_limit];

    // for (int i = 0; i < worker_limit; i++) {
    //     pthread_create(&worker_thread_pool[i], NULL, foo, NULL);
    // }

    // Προετοιµάζει επίσης τον συγχρονισµό καταλόγων που καθορίζονται στο config_file:
    // Αρχικά συνδέεται στο source_host:source_port όπου τρέχει ένας nfs_client και στέλνει µια εντολή:

    FILE *configFp = fopen(config_file, "r");         // Open the config file for reading.
    if (configFp == NULL) {
        perror("Error opening file.");
        return 1;
    }

    // Initialize variables.
    char line[1024];

    while (fgets(line, sizeof(line), configFp)) { // Read the config file.
        line[strcspn(line, "\n")] = 0;

        char *source_dir   = strtok(line, " @:");
        char *source_host  = strtok(NULL, " @:");
        char *source_port  = strtok(NULL, " @:");
        char *target_dir   = strtok(NULL, " @:");
        char *target_host  = strtok(NULL, " @:");
        char *target_port  = strtok(NULL, " @:");


        if (check_dir(source_dir) && check_dir(target_dir)) {
            watchDir* curr = create_dir(source_dir, source_host, source_port, target_dir, target_host, target_port);
            insert_watchDir(table, curr);
        }
    }
    print_hash_table(table);
    
    // Στο οποίο θα δέχεται µηνύµατα επικοινωνίας από το nfs_console.
    // Keep connection open.
    // while (1) {
    //     memset(buffer, 0, sizeof(buffer));
    //     int bytes = read(clientfd, buffer, sizeof(buffer));
    //     if (bytes <= 0) 
    //         break;

    //     // 1. Τυπώνει στην οθόνη,
    //     char* command = strtok(buffer," ");
    //     char* source = strtok(NULL," ");
    //     char* target = strtok(NULL," ");
    //     if (strcmp(command, "add") == 0) {
    //         printf("[%s] Added file: %s -> %s\n", getTime(), source, target);
    //     } else if (strcmp(command, "cancel") == 0) {
    //         printf("[%s] Synchronization stopped for: %s\n", getTime(), source);
    //     } else {
    //         printf("[%s] Shutting down manager...\n", getTime());
    //         printf("[%s] Waiting for all active workers to finish.\n", getTime());
    //         printf("[%s] Processing remaining queued tasks.\n", getTime());
    //         printf("[%s] Manager shutdown complete.\n", getTime());
    //         break;
    //     }
    //     // 2. Στέλνει στο nfs_console, not done
    //     write(clientfd, "ACK from server", strlen("ACK from server"));
    //     // 3. Γράφει στο manager-log-file. not done
    // }
    
    // close(clientfd);
    // close(serverfd);
    return EXIT_SUCCESS;
}