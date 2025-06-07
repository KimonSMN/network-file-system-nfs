#ifndef QUEUE
#define QUEUE

#include <time.h>
#include <stdlib.h>

typedef struct node{
    char*   source_host;
    char*   source_port;
    char*   source_dir;

    char*   target_host;
    char*   target_port;
    char*   target_dir;
    
    char* filename;
    char* operation;
    struct node* next;
} node;


typedef struct queue{
    node* head;
    node* tail;
    int size;
} queue;

/* Initializes node struct and returns the node. */
node* init_node(char* source_dir, char* source_host, char* source_port, char* target_dir, char* target_host, char* target_port, char* filename, char* operation);
/* Free memory of allocated `node* job` */
void destroy_node (node* job);

/* Initializes the queue struct and returns the queue. */
queue* init_queue();

/* Enqueue `node* job` to the `queue* q` */
int enqueue(queue* q, node* job);

/* Dequeue from the `queue* q` */
node* dequeue(queue* q);

/* Return 1 If `queue* q` is empty.
    Return 0 if it's not empty. */
int isEmpty(queue* q);

#endif