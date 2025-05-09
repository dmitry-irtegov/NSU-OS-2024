#ifndef TASK25_H
#define TASK25_H

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define MAX_QUEUE_SIZE 10
#define MAX_MSG_LENGTH 80

typedef struct {
    char messages[MAX_QUEUE_SIZE][MAX_MSG_LENGTH + 1];
    int head;
    int tail;
    int count;
    sem_t empty;
    sem_t full;
    pthread_mutex_t mutex;
    int dropped;
} queue;

void mymsginit(queue *q);
void mymsgdestroy(queue *q);
void mymsqdrop(queue *q);
int mymsgput(queue *q, char *msg);
int mymsgget(queue *q, char *buf, size_t bufsize);

#endif