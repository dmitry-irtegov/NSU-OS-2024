#pragma once
#include <pthread.h>
#include <semaphore.h>
#define MSG_COUNT 10
#define BUFF_SIZE 80
#define TIMEOUT_MS (500)


typedef struct {
    char msgs[MSG_COUNT][BUFF_SIZE + 1];
    unsigned int next_free_index, next_msg_index;
    pthread_mutex_t lock;
    sem_t avail_msg, avail_space;
    
    /* for mymsgdrop() */
    volatile unsigned char is_dropped;
} Queue;

void mymsginit(Queue* queue);
void mymsgdestroy(Queue* queue);
int mymsgput(Queue* queue, char* msg);
int mymsgget(Queue* queue, char* buf, size_t bufsize); 
void mymsgdrop(Queue* queue);

