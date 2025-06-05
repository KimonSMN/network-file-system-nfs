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

queue* q;
pthread_mutex_t mutex;
pthread_cond_t cond;


void* worker_thread(void* arg) {

    while (1) {
        pthread_mutex_lock(&mutex);

        while (isEmpty(q)) {
            pthread_cond_wait(&cond, &mutex);
        }

        node* node = dequeue(q);
        pthread_mutex_unlock(&mutex);

        if (node == NULL || node->filename == NULL || node->source_dir == NULL) {
            perror("Faulty Job");
            continue;
        }

        int source_socket = myconnect(node->source_host, atoi(node->source_port));
        if (source_socket < 0) {
            perror("Connection failed");
            continue;
        }
    
        char command[4096];
        snprintf(command, sizeof(command), "PULL %s/%s\n", node->source_dir,node->filename);
        write(source_socket, command, strlen(command));
        memset(command, 0, sizeof(command));

        int target_socket = myconnect(node->target_host, atoi(node->target_port));
        if (target_socket < 0) {
            perror("Connection failed");
            continue;
        }

        char buffer[4096] = {0};
        ssize_t bytes_read;
        
        snprintf(command, sizeof(command), "PUSH %s/%s \n", node->target_dir, node->filename);
        write(target_socket, command, strlen(command));

        char chunk_buf[1024];
        while ((bytes_read = read(source_socket, buffer, sizeof(buffer))) > 0) {
            // Send chunk that was read. (chunk = bytes_read).
            snprintf(chunk_buf, sizeof(chunk_buf), "%ld\n", bytes_read);
            write(target_socket, chunk_buf, strlen(chunk_buf)); // Chunk
            write(target_socket, buffer, bytes_read);           // Data
            printf("[+]Wrote to client: Chunk: %s | Data: %s\n", chunk_buf,buffer);
        }   

        close(source_socket);
        close(target_socket);

    }
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

    if (manager_logfile == NULL || config_file == NULL || worker_limit < MAX_WORKERS || port_number <= 0 || bufferSize <= 0) {
        printf("Usage: ./nfs_manager -l <manager_logfile> -c <config_file> -n <worker_limit> -p <port_number> -b <bufferSize>\n");
        return 1;
    }

    // Δηµιουργεί ένα socket στο port που δόθηκε.

    int serverfd, consolefd;
    struct sockaddr_in servaddr;

    if ((serverfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    int opt = 1;
    setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(port_number);

    bind(serverfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    listen(serverfd, 5);

    consolefd = accept(serverfd, NULL, NULL);

    // We wait until nfs_console joins......
    // Then we can continue.

    char buffer[1024];

    // Ακολούθως, ετοιµάζει τις διάφορες δοµές δεδοµένων που θα χρειαστεί για το συγχρονισµό των καταλόγων.
    hashTable* table = init_hash_table();   // Initialize the hash-table.
    q = init_queue();                       // Initialize the queue.
    
    // Ετοιµάζει και δηµιουργεί ένα worker thread pool.

    pthread_t worker_thread_pool[worker_limit];
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    for (int i = 0; i < worker_limit; i++) {
        pthread_create(&worker_thread_pool[i], NULL, worker_thread, NULL);
    }

    // Προετοιµάζει επίσης τον συγχρονισµό καταλόγων που καθορίζονται στο config_file:
    // Αρχικά συνδέεται στο source_host:source_port όπου τρέχει ένας nfs_client και στέλνει µια εντολή:

    FILE *configFp = fopen(config_file, "r");         // Open the config file for reading.
    if (configFp == NULL) {
        return 1;
    }

    FILE *logfileFp = fopen(manager_logfile, "w");
    if (logfileFp == NULL) {
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

            // creatre a tcp connection
            // Connection with nfs_client(s)
            int sockfd;
            struct sockaddr_in servaddr;

            // Create socket
            sockfd = socket(AF_INET, SOCK_STREAM, 0);

            int opt = 1;
            setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
            memset(&servaddr, 0, sizeof(servaddr));
            servaddr.sin_family = AF_INET;
            servaddr.sin_port = htons(atoi(source_port));

            inet_pton(AF_INET, source_host, &servaddr.sin_addr);
            if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
                perror("Connection failed");
                close(sockfd);
                continue;
            }

            char command[512];
            snprintf(command, sizeof(command), "LIST %s\n", source_dir);
            write(sockfd, command, sizeof(command));
            char new[1024];
            memset(new, 0, sizeof(new));

            read(sockfd, new, sizeof(new));
            char* file = strtok(new, "\n"); // get first file from the buffer "file$\nfile$\n."
            while (file != NULL) {

                if (strcmp(file, ".\0") == 0) break; // end of file list
                if (strlen(file) == 0) {
                    file = strtok(NULL, "\n");
                    continue;
                }

                // Τυπώνει στην οθόνη. Γράφει στο manager-log-file
                printf_fprintf(logfileFp,"[%s] Added file: %s/%s@%s:%s -> %s/%s@%s:%s\n", getTime(), source_dir, file, source_host, source_port, target_dir, file, target_host, target_port);
                fflush(logfileFp);
                // Στέλνει στο nfs_console
                char consolebuffer[1024];
                snprintf(consolebuffer,1024,"[%s] Added file: %s/%s@%s:%s -> %s/%s@%s:%s\n", getTime(), source_dir, file, source_host, source_port, target_dir,file , target_host, target_port );
                write(consolefd, consolebuffer, strlen(consolebuffer));
                
                node* job = init_node(source_dir, source_host, source_port, target_dir, target_host, target_port, file, "PUSH");
                pthread_mutex_lock(&mutex);
                enqueue(q, job);
                pthread_cond_broadcast(&cond); 
                pthread_mutex_unlock(&mutex);

                file = strtok(NULL ,"\n");
            }
            close(sockfd);
        }
    }
    // print_hash_table(table);

    // Στο οποίο θα δέχεται µηνύµατα επικοινωνίας από το nfs_console.
    // Keep connection open.
    while (1) {

        memset(buffer, 0, sizeof(buffer));
        int bytes = read(consolefd, buffer, sizeof(buffer));
        if (bytes <= 0) {
            continue;;
        }

        // 1. Τυπώνει στην οθόνη,
        char* command = strtok(buffer," ");
        char* source = strtok(NULL," ");
        char* target = strtok(NULL," ");
        if (strcmp(command, "add") == 0) {
            printf("[%s] Added file: %s -> %s\n", getTime(), source, target);
        } else if (strcmp(command, "cancel") == 0) {
            printf("[%s] Synchronization stopped for: %s\n", getTime(), source);
        } else {
            printf("[%s] Shutting down manager...\n", getTime());
            printf("[%s] Waiting for all active workers to finish.\n", getTime());
            printf("[%s] Processing remaining queued tasks.\n", getTime());
            printf("[%s] Manager shutdown complete.\n", getTime());
            break;
        }
        // 2. Στέλνει στο nfs_console, not done
        write(consolefd, "ACK from server", strlen("ACK from server"));
        // 3. Γράφει στο manager-log-file. not done

    }

    fclose(configFp);
    fclose(logfileFp);
    close(consolefd);
    close(serverfd);
    return EXIT_SUCCESS;
}