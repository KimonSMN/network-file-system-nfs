#include <stdio.h>
#include <string.h>

#include "queue.h"

node* init_node(char* source_dir, char* source_host, char* source_port, char* target_dir, char* target_host, char* target_port, char* filename, char* operation){
    node* job = malloc(sizeof(struct node));
    if (job == NULL) return NULL;
    job->source_dir = strdup(source_dir);
    job->source_host = strdup(source_host);
    job->source_port = strdup(source_port);

    job->target_dir = strdup(target_dir);
    job->target_host = strdup(target_host);
    job->target_port = strdup(target_port);
    
    job->filename = strdup(filename);
    job->operation = strdup(operation);
    job->next = NULL;

    return job;
}

void destroy_node(node* job){
    if(job == NULL) return;

    free(job->source_dir);
    free(job->source_host);
    free(job->source_port);

    free(job->target_dir);
    free(job->target_host);
    free(job->target_port);

    free(job->filename);
    free(job->operation);
    free(job);
}

queue* init_queue(){
    queue* q = malloc(sizeof(struct queue));
    if(q == NULL) return NULL;
    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
    return q;
}

int enqueue(queue* q, node* job){
    if (q == NULL) return 1;
    if (job == NULL) return 1;
    
    job->next = NULL;
    if (q->tail != NULL)   // if there is a tail connect the node next to it
        q->tail->next = job;
    
    q->tail = job;
    if(q->head == NULL)
        q->head = job;
    
    q->size++;
    return 0;
}

node* dequeue(queue* q){
    if(q->head == NULL) return NULL;
    
    node* job = q->head;
    q->head = job->next;

    if(q->head == NULL) 
        q->tail = NULL;
    
    if (q->size > 0)
        q->size--;

    return job;
}

int isEmpty(queue* q){
   
    return q->head == NULL;
}

void destroy_queue(queue* q) {
    if (q == NULL) return;
    
    node* curr = q->head;

    while (curr != NULL) {
        node* temp = curr->next;
        destroy_node(curr);
        curr = temp;
    }

    free(q);
}