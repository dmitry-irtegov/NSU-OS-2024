#ifndef TASK25_COND_H
#define TASK25_COND_H

#include <pthread.h>

#define MAX_QUEUE_SIZE 10
#define MAX_MSG_LENGTH 80

typedef struct {
    char messages[MAX_QUEUE_SIZE][MAX_MSG_LENGTH + 1];
    int head;
    int tail;
    int count;
    pthread_mutex_t mutex;
    pthread_cond_t not_full;
    pthread_cond_t not_empty;
    int dropped;
} queue;

void mymsginit(queue *q);
void mymsgdestroy(queue *q);
void mymsqdrop(queue *q);
int mymsgput(queue *q, char *msg);
int mymsgget(queue *q, char *buf, size_t bufsize);

#endif