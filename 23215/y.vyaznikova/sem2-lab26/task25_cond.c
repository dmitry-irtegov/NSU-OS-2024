#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "task25_cond.h"

void mymsginit(queue *q) {
    if (q == NULL) {
        return;
    }

    q->head = 0;
    q->tail = 0;
    q->count = 0;
    q->dropped = 0;
    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->not_full, NULL);
    pthread_cond_init(&q->not_empty, NULL);
}

void mymsgdestroy(queue *q) {
    if (q == NULL) {
        return;
    }

    pthread_mutex_destroy(&q->mutex);
    pthread_cond_destroy(&q->not_full);
    pthread_cond_destroy(&q->not_empty);
}

void mymsqdrop(queue *q) {
    if (q == NULL) {
        return;
    }

    pthread_mutex_lock(&q->mutex);
    q->dropped = 1;
    pthread_cond_broadcast(&q->not_full);
    pthread_cond_broadcast(&q->not_empty);
    pthread_mutex_unlock(&q->mutex);
}

int mymsgput(queue *q, char *msg) {
    if (q == NULL || msg == NULL) {
        return 0;
    }

    pthread_mutex_lock(&q->mutex);

    if (q->dropped) {
        pthread_mutex_unlock(&q->mutex);
        return 0;
    }

    while (q->count >= MAX_QUEUE_SIZE && !q->dropped) {
        pthread_cond_wait(&q->not_full, &q->mutex);
    }

    if (q->dropped) {
        pthread_mutex_unlock(&q->mutex);
        return 0;
    }

    strncpy(q->messages[q->tail], msg, MAX_MSG_LENGTH);
    q->messages[q->tail][MAX_MSG_LENGTH] = '\0';
    
    size_t len = strlen(q->messages[q->tail]);
    q->tail = (q->tail + 1) % MAX_QUEUE_SIZE;
    q->count++;

    pthread_cond_signal(&q->not_empty);
    pthread_mutex_unlock(&q->mutex);

    return len;
}

int mymsgget(queue *q, char *buf, size_t bufsize) {
    if (q == NULL || buf == NULL || bufsize == 0) {
        return 0;
    }

    pthread_mutex_lock(&q->mutex);

    if (q->dropped) {
        pthread_mutex_unlock(&q->mutex);
        return 0;
    }

    while (q->count == 0 && !q->dropped) {
        pthread_cond_wait(&q->not_empty, &q->mutex);
    }

    if (q->dropped) {
        pthread_mutex_unlock(&q->mutex);
        return 0;
    }

    strncpy(buf, q->messages[q->head], bufsize - 1);
    buf[bufsize - 1] = '\0';
    
    size_t len = strlen(buf);
    q->head = (q->head + 1) % MAX_QUEUE_SIZE;
    q->count--;

    pthread_cond_signal(&q->not_full);
    pthread_mutex_unlock(&q->mutex);

    return len;
}