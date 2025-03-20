#include "queue.h"
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#define THREADS_MSG_COUNT 20


void mymsginit(Queue* queue) {
    sem_init(&queue->avail_msg, 0, 0);
    sem_init(&queue->avail_space, 0, MSG_COUNT);
    queue->next_free_index= queue->next_msg_index = 0; 
    pthread_mutex_init(&queue->lock, NULL);
}

void mymsgdestroy(Queue* queue) {
    pthread_mutex_destroy(&queue->lock);
    sem_destroy(&queue->avail_msg);
    sem_destroy(&queue->avail_space);
}

int mymsgput(Queue* queue, char* msg) {
    pthread_mutex_lock(&queue->lock);
    if (queue->is_dropped) {
        return 0;
    }
    pthread_mutex_unlock(&queue->lock);
    
    sem_wait(&queue->avail_space);

    pthread_mutex_lock(&queue->lock);
    if (queue->is_dropped) {
        return 0;
    }
    unsigned long msg_len = strlen(msg);
    unsigned long result_len = (BUFF_SIZE < msg_len) ? BUFF_SIZE : msg_len;
    int new_msg_index = queue->next_free_index;
    strncpy(queue->msgs[new_msg_index], msg, result_len);
    queue->msgs[new_msg_index][result_len] = 0;
    queue->next_free_index = (new_msg_index + 1) % MSG_COUNT;

    pthread_mutex_unlock(&queue->lock);
    
    sem_post(&queue->avail_msg);
    return result_len;
}

int mymsgget(Queue* queue, char* buf, size_t bufsize) {
    pthread_mutex_lock(&queue->lock);
    if (queue->is_dropped) {
        return 0;
    }
    pthread_mutex_unlock(&queue->lock);

    sem_wait(&queue->avail_msg);
    
    pthread_mutex_lock(&queue->lock);
    if (queue->is_dropped) {
        return 0;
    } 
    unsigned int msg_index = queue->next_msg_index;
    unsigned long msg_len = strlen(queue->msgs[msg_index]);
    unsigned long result_len = (bufsize < msg_len) ? bufsize : msg_len;
    strncpy(buf, queue->msgs[msg_index], result_len);
    queue->next_msg_index = (msg_index + 1) % MSG_COUNT;
    
    pthread_mutex_unlock(&queue->lock);

    sem_post(&queue->avail_space);
    return result_len;
}

void unblock_sem_waiters(sem_t* sem) {
    sched_yield();
    int waiters_count = 0;

    sem_getvalue(sem, &waiters_count);

    while (waiters_count < 0) {
    printf("val: %d\n", waiters_count);
        sem_post(sem);
        sem_getvalue(sem, &waiters_count);
    } 
}

void mymsgdrop(Queue* queue) {
    pthread_mutex_lock(&queue->lock);
    
    queue->is_dropped = 1;
   
    unblock_sem_waiters(&queue->avail_space);
    sem_destroy(&queue->avail_space);
    unblock_sem_waiters(&queue->avail_msg);
    sem_destroy(&queue->avail_msg);

    pthread_mutex_unlock(&queue->lock);
}
