#include "queue.h"
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <time.h>
#include <errno.h>

void mymsginit(Queue* queue) {
    sem_init(&queue->avail_msg, 0, 0);
    sem_init(&queue->avail_space, 0, MSG_COUNT);
    queue->is_dropped = queue->next_free_index = queue->next_msg_index = 0;
    pthread_mutex_init(&queue->lock, NULL);
}

void mymsgdestroy(Queue* queue) {
    pthread_mutex_destroy(&queue->lock);
    sem_destroy(&queue->avail_msg);
    sem_destroy(&queue->avail_space);
}

void set_timeout(struct timespec* time) {
    clock_gettime(CLOCK_REALTIME, time);
    time->tv_nsec += TIMEOUT_MS * 1000000L;
    time->tv_sec = time->tv_nsec / 1000000000L;
    time->tv_nsec = time->tv_nsec % 1000000000L;
}

int mymsgput(Queue* queue, char* msg) {
    struct timespec t;
    do {
        if (queue->is_dropped) {
            return 0;
        }
        set_timeout(&t);
    } while(sem_timedwait(&queue->avail_space, &t) == -1 && errno == ETIMEDOUT);

    pthread_mutex_lock(&queue->lock);
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
    struct timespec t;
    do {
        if (queue->is_dropped) {
            return 0;
        }
        set_timeout(&t);
    } while(sem_timedwait(&queue->avail_msg, &t) == -1 && errno == ETIMEDOUT);

    pthread_mutex_lock(&queue->lock);
    unsigned int msg_index = queue->next_msg_index;
    unsigned long msg_len = strlen(queue->msgs[msg_index]);
    unsigned long result_len = (bufsize < msg_len) ? bufsize : msg_len;
    strncpy(buf, queue->msgs[msg_index], result_len);
    queue->next_msg_index = (msg_index + 1) % MSG_COUNT;
    
    pthread_mutex_unlock(&queue->lock);

    sem_post(&queue->avail_space);
    return result_len;
}

void unblock_sem_waiters(sem_t* sem, sem_t* count_wait_sem) {
    sched_yield();

    int val = 0;
    while (sem_getvalue(sem, &val) == 0 && val == 0) {
        sem_post(sem);
    }
}

void mymsgdrop(Queue* queue) {
    queue->is_dropped = 1;

    // pthread_mutex_lock(&queue->lock);
   
    // unblock_sem_waiters(&queue->avail_space, &queue->curr_wait_space);
    // unblock_sem_waiters(&queue->avail_msg, &queue->curr_wait_msg);

    // pthread_mutex_unlock(&queue->lock);
    printf("queueu was dropped\n");
}
