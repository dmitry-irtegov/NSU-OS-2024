#pragma once
#include <pthread.h>
#include <semaphore.h>
#define MSG_COUNT 10
#define BUFF_SIZE 80

typedef struct {
    char msgs[MSG_COUNT][BUFF_SIZE + 1];
    volatile unsigned int next_free_index, next_msg_index, msg_count,
        free_cells_count;
    pthread_mutex_t lock;
    pthread_cond_t new_msg_cond, new_cell_cond;
    volatile unsigned char is_dropped;
} Queue;

void mymsginit(Queue* queue);
void mymsgdestroy(Queue* queue);
int mymsgput(Queue* queue, char* msg);
int mymsgget(Queue* queue, char* buf, size_t bufsize);
void mymsgdrop(Queue* queue);
