#ifndef QUEUE_H
#define QUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define MAX_QUEUE_SIZE 10
#define MAX_MSG_LENGTH 80

typedef struct msg_node {
    char data[MAX_MSG_LENGTH + 1];
    struct msg_node *next;
} msg_node;

typedef struct {
    msg_node *head;
    msg_node *tail;
    int count;
    sem_t empty;
    sem_t full;
    pthread_mutex_t mutex;
    int dropped;
} queue;

void mymsginit(queue *q);
void mymsgdestroy(queue *q);
void mymsgdrop(queue *q);
int mymsgput(queue *q, char *msg);
int mymsgget(queue *q, char *buf, size_t bufsize);

#endif