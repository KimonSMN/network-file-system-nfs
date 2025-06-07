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
    sleep(4);
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
    

        write_pull(source_socket, node->source_dir, node->filename);
        int target_socket = myconnect(node->target_host, atoi(node->target_port));
        if (target_socket < 0) {
            perror("Connection failed");
            continue;
        }
        
        char command[4096] = {0};
        snprintf(command, sizeof(command), "PUSH %s/%s \n", node->target_dir, node->filename);
        write(target_socket, command, strlen(command));

        char chunk_buf[1024];
        char buffer[4096] = {0};
        ssize_t bytes_read;

        while ((bytes_read = read(source_socket, buffer, sizeof(buffer))) > 0) {
            // Send chunk that was read. (chunk = bytes_read).
            snprintf(chunk_buf, sizeof(chunk_buf), "%ld\n", bytes_read);
            write(target_socket, chunk_buf, strlen(chunk_buf)); // Chunk
            write(target_socket, buffer, bytes_read);           // Data
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

    int server_fd, console_fd;
    struct sockaddr_in servaddr;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        exit(1);
    
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(port_number);

    bind(server_fd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    listen(server_fd, 5);

    console_fd = accept(server_fd, NULL, NULL);

    // Initialization:
    hashTable* table = init_hash_table();
    q = init_queue();
    pthread_t worker_thread_pool[worker_limit];
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    for (int i = 0; i < worker_limit; i++) {
        pthread_create(&worker_thread_pool[i], NULL, worker_thread, NULL);
    }

    FILE *configFp = fopen(config_file, "r");
    if (configFp == NULL) {
        return 1;
    }

    FILE *logfileFp = fopen(manager_logfile, "w");
    if (logfileFp == NULL) {
        return 1;
    }

    char line[1024];
    while (fgets(line, sizeof(line), configFp)) { // Read the config file.
        line[strcspn(line, "\n")] = 0;

        char *source_dir   = strtok(line, " @:");
        char *source_host  = strtok(NULL, " @:");
        char *source_port  = strtok(NULL, " @:");
        char *target_dir   = strtok(NULL, " @:");
        char *target_host  = strtok(NULL, " @:");
        char *target_port  = strtok(NULL, " @:");

        watchDir* curr = create_dir(source_dir, source_host, source_port, target_dir, target_host, target_port);
        insert_watchDir(table, curr);

        // Connection with nfs_client(s)
        int client_socket = myconnect(source_host, atoi(source_port));
        write_list(client_socket, source_dir);  // Send LIST command to client.
        
            // Read from client the array requested with LIST.
        char list_array[1024] = {0};
        read(client_socket, list_array, sizeof(list_array));

        char* file = strtok(list_array, "\n");
        while (file != NULL) {  // For every file in the array.

            if (strcmp(file, ".\0") == 0) // Signal EOF
                break;

            if (q->size >= bufferSize) {
                file = strtok(NULL ,"\n");
                continue;
            }

            // Prints output to the screen & to manager-log-file.
            printf_fprintf(logfileFp,"[%s] Added file: %s/%s@%s:%s -> %s/%s@%s:%s\n", getTime(), source_dir, file, source_host, source_port, target_dir, file, target_host, target_port);
            // Sends message to nfs_console
            char console_buf[1024];
            snprintf(console_buf,1024,"[%s] Added file: %s/%s@%s:%s -> %s/%s@%s:%s\n", getTime(), source_dir, file, source_host, source_port, target_dir,file , target_host, target_port );
            write(console_fd, console_buf, strlen(console_buf));
            // Setup job
      
            node* job = init_node(source_dir, source_host, source_port, target_dir, target_host, target_port, file, "PUSH");
            pthread_mutex_lock(&mutex);
            enqueue(q, job);
            pthread_cond_broadcast(&cond); 
            pthread_mutex_unlock(&mutex);

            file = strtok(NULL ,"\n");
        }
        close(client_socket);
        
    }
    char buffer[1024];
    // Keep connection open.
    while (1) {

        memset(buffer, 0, sizeof(buffer));
        int bytes = read(console_fd, buffer, sizeof(buffer));
        if (bytes <= 0) {   // If there isn't something to read continue.
            continue;
        }

        char* command = strtok(buffer," ");
        if (command == NULL) 
            continue;

        if (strcmp(command, "shutdown") == 0) {
            fflush(stdout);
            printf("[%s] Shutting down manager...\n", getTime());
            printf("[%s] Waiting for all active workers to finish.\n", getTime());
            printf("[%s] Processing remaining queued tasks.\n", getTime());
            printf("[%s] Manager shutdown complete.\n", getTime());
            break;
        } else if (strcmp(command, "cancel") == 0) {
            char *source_dir  = strtok(NULL, " @:");
            char *source_host = strtok(NULL, " @:");
            char *source_port = strtok(NULL, " @:");

            watchDir* curr = find_watchDir(table, source_dir);  // have to change to include full format source@host:port
            if (curr) {
                printf_fprintf_write(console_fd, logfileFp,"[%s] Synchronization stopped for: %s@%s:%s\n", getTime(), source_dir,source_host,source_port);
                fflush(stdout);
                curr->syncing = 0; // doesnt actually do anything rn.
            } else {
                printf_write(console_fd,"[%s] Directory not being synchronized: %s\n", getTime(), source_dir);
                fflush(stdout);
            }
            
        } else if (strcmp(command, "add") == 0) {
            char *source_dir  = strtok(NULL, " @:");
            char *source_host = strtok(NULL, " @:");
            char *source_port = strtok(NULL, " @:");
            char *target_dir  = strtok(NULL, " @:");
            char *target_host = strtok(NULL, " @:");
            char *target_port = strtok(NULL, " @:");
            watchDir* curr = create_dir(source_dir, source_host, source_port, target_dir, target_host, target_port);
            insert_watchDir(table, curr);

            // Connection with nfs_client(s)
            int client_socket = myconnect(source_host, atoi(source_port));
            write_list(client_socket, source_dir);  // Send LIST command to client.
            
             // Read from client the array requested with LIST.
            char list_array[1024] = {0};
            read(client_socket, list_array, sizeof(list_array));

            char* file = strtok(list_array, "\n");
            while (file != NULL) {  // For every file in the array.

                if (strcmp(file, ".\0") == 0) // Signal EOF
                    break;

                // Prints output to the screen & to manager-log-file & sends it to console.
                printf_fprintf_write(console_fd,logfileFp,"[%s] Added file: %s/%s@%s:%s -> %s/%s@%s:%s\n", getTime(), source_dir, file, source_host, source_port, target_dir, file, target_host, target_port);
                
                // Setup job
                node* job = init_node(source_dir, source_host, source_port, target_dir, target_host, target_port, file, "PUSH");
                pthread_mutex_lock(&mutex);
                enqueue(q, job);
                pthread_cond_broadcast(&cond); 
                pthread_mutex_unlock(&mutex);

                file = strtok(NULL ,"\n");
            }
            close(client_socket);

            fflush(stdout);
        }
    }

    fclose(configFp);
    fclose(logfileFp);
    close(console_fd);
    close(server_fd);
    return EXIT_SUCCESS;
}
