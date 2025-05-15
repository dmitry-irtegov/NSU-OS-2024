#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include "task25_sem.h"

void mymsginit(queue *q) {
    if (q == NULL) {
        return;
    }

    q->head = 0;
    q->tail = 0;
    q->count = 0;
    q->dropped = 0;

    sem_init(&q->empty, 0, MAX_QUEUE_SIZE);
    sem_init(&q->full, 0, 0);
    pthread_mutex_init(&q->mutex, NULL);
}

void mymsgdestroy(queue *q) {
    if (q == NULL) {
        return;
    }

    sem_destroy(&q->empty);
    sem_destroy(&q->full);
    pthread_mutex_destroy(&q->mutex);
}

void mymsqdrop(queue *q) {
    if (q == NULL) {
        return;
    }

    pthread_mutex_lock(&q->mutex);
    q->dropped = 1;
    pthread_mutex_unlock(&q->mutex);

    int empty_value, full_value;
    int max_attempts = 100;

    while (max_attempts--) {
        sem_getvalue(&q->empty, &empty_value);
        sem_getvalue(&q->full, &full_value);

        if (empty_value > 0 && full_value > 0) {
            break;
        }

        if (empty_value <= 0) sem_post(&q->empty);
        if (full_value <= 0) sem_post(&q->full);

        nanosleep(&(struct timespec){0, 1000000}, NULL);
    }
}

int mymsgput(queue *q, char *msg) {
    if (q == NULL || msg == NULL) {
        return 0;
    }

    if (sem_wait(&q->empty) != 0) {
        return 0;
    }

    pthread_mutex_lock(&q->mutex);

    if (q->dropped) {
        pthread_mutex_unlock(&q->mutex);
        sem_post(&q->empty);
        return 0;
    }

    strncpy(q->messages[q->tail], msg, MAX_MSG_LENGTH);
    q->messages[q->tail][MAX_MSG_LENGTH] = '\0';
    
    size_t len = strlen(q->messages[q->tail]);
    q->tail = (q->tail + 1) % MAX_QUEUE_SIZE;
    q->count++;

    pthread_mutex_unlock(&q->mutex);
    sem_post(&q->full);

    return len;
}

int mymsgget(queue *q, char *buf, size_t bufsize) {
    if (q == NULL || buf == NULL || bufsize == 0) {
        return 0;
    }

    if (sem_wait(&q->full) != 0) {
        return 0;
    }

    pthread_mutex_lock(&q->mutex);

    if (q->dropped) {
        pthread_mutex_unlock(&q->mutex);
        sem_post(&q->full);
        return 0;
    }

    strncpy(buf, q->messages[q->head], bufsize - 1);
    buf[bufsize - 1] = '\0';
    
    size_t len = strlen(buf);
    q->head = (q->head + 1) % MAX_QUEUE_SIZE;
    q->count--;

    pthread_mutex_unlock(&q->mutex);
    sem_post(&q->empty);

    return len;
}
