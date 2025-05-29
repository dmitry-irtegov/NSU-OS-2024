#include "queue.h"

#include <assert.h>
#include <pthread.h>
#include <sched.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

void mymsginit(Queue* queue) {
    pthread_cond_init(&queue->new_msg_cond, NULL);
    pthread_cond_init(&queue->new_cell_cond, NULL);
    queue->msg_count = queue->is_dropped = queue->next_free_index =
        queue->next_msg_index = 0;
    queue->free_cells_count = 10;
    pthread_mutex_init(&queue->lock, NULL);
}

void mymsgdestroy(Queue* queue) {
    pthread_mutex_destroy(&queue->lock);
    pthread_cond_destroy(&queue->new_msg_cond);
    pthread_cond_destroy(&queue->new_cell_cond);
}

int mymsgput(Queue* queue, char* msg) {
    pthread_mutex_lock(&queue->lock);
    if (queue->is_dropped) {
        pthread_mutex_unlock(&queue->lock);
        return 0;
    }
    while (queue->free_cells_count <= 0) {
        pthread_cond_wait(&queue->new_cell_cond, &queue->lock);
        if (queue->is_dropped) {
            pthread_mutex_unlock(&queue->lock);
            return 0;
        }
    }
    queue->free_cells_count--;

    unsigned long msg_len = strlen(msg);
    unsigned long result_len = (BUFF_SIZE < msg_len) ? BUFF_SIZE : msg_len;
    int new_msg_index = queue->next_free_index;
    strncpy(queue->msgs[new_msg_index], msg, result_len);
    queue->msgs[new_msg_index][result_len] = 0;
    queue->next_free_index = (new_msg_index + 1) % MSG_COUNT;

    printf("putted \"%s\"\n", msg);
    fflush(stdout);
    queue->msg_count++;
    pthread_cond_signal(&queue->new_msg_cond);
    pthread_mutex_unlock(&queue->lock);
    return result_len;
}

int mymsgget(Queue* queue, char* buf, size_t bufsize) {
    pthread_mutex_lock(&queue->lock);
    if (queue->is_dropped) {
        pthread_mutex_unlock(&queue->lock);
        return 0;
    }
    while (queue->msg_count <= 0) {
        pthread_cond_wait(&queue->new_msg_cond, &queue->lock);
        if (queue->is_dropped) {
            pthread_mutex_unlock(&queue->lock);
            return 0;
        }
    }
    queue->msg_count--;

    unsigned int msg_index = queue->next_msg_index;
    unsigned long msg_len = strlen(queue->msgs[msg_index]);
    unsigned long result_len = (bufsize < msg_len) ? bufsize : msg_len;
    strncpy(buf, queue->msgs[msg_index], result_len);
    buf[result_len] = 0;
    queue->next_msg_index = (msg_index + 1) % MSG_COUNT;

    printf("getted \"%s\"\n", buf);
    fflush(stdout);
    queue->free_cells_count++;
    pthread_cond_signal(&queue->new_cell_cond);
    pthread_mutex_unlock(&queue->lock);
    return result_len;
}

void mymsgdrop(Queue* queue) {
    pthread_mutex_lock(&queue->lock);
    queue->is_dropped = 1;
    pthread_cond_broadcast(&queue->new_cell_cond);
    pthread_cond_broadcast(&queue->new_msg_cond);
    pthread_mutex_unlock(&queue->lock);
    printf("queueu was dropped\n");
}
