#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "queue.h"

void mymsginit(queue *q) {
    if (q == NULL) return;

    q->head = NULL;
    q->tail = NULL;
    q->count = 0;
    q->dropped = 0;

    if (sem_init(&q->empty, 0, MAX_QUEUE_SIZE) != 0) {
        perror("sem_init empty failed");
        return;
    }

    if (sem_init(&q->full, 0, 0) != 0) {
        perror("sem_init full failed");
        sem_destroy(&q->empty);
        return;
    }

    if (pthread_mutex_init(&q->mutex, NULL) != 0) {
        perror("mutex_init failed");
        sem_destroy(&q->empty);
        sem_destroy(&q->full);
        return;
    }
}

void mymsgdestroy(queue *q) {
    if (q == NULL) return;

    msg_node *current = q->head;
    while (current != NULL) {
        msg_node *temp = current;
        current = current->next;
        free(temp);
    }

    sem_destroy(&q->empty);
    sem_destroy(&q->full);
    pthread_mutex_destroy(&q->mutex);
}

void mymsgdrop(queue *q) {
    if (q == NULL) return;

    pthread_mutex_lock(&q->mutex);
    q->dropped = 1;

    while (sem_trywait(&q->empty) == 0) ;
    while (sem_trywait(&q->full) == 0) ;

    msg_node *current = q->head;
    while (current != NULL) {
        msg_node *temp = current;
        current = current->next;
        free(temp);
    }
    q->head = NULL;
    q->tail = NULL;
    q->count = 0;

    pthread_mutex_unlock(&q->mutex);
}


int mymsgput(queue *q, char *msg) {
    if (q == NULL || msg == NULL) return 0;

    if (sem_wait(&q->empty) != 0) return 0;

    pthread_mutex_lock(&q->mutex);
    if (q->dropped) {
        pthread_mutex_unlock(&q->mutex);
        sem_post(&q->empty);
        return 0;
    }

    msg_node *new_node = malloc(sizeof(msg_node));
    if (new_node == NULL) {
        pthread_mutex_unlock(&q->mutex);
        sem_post(&q->empty);
        return 0;
    }

    strncpy(new_node->data, msg, MAX_MSG_LENGTH);
    new_node->data[MAX_MSG_LENGTH] = '\0';
    new_node->next = NULL;

    if (q->tail == NULL) {
        q->head = new_node;
        q->tail = new_node;
    } else {
        q->tail->next = new_node;
        q->tail = new_node;
    }
    q->count++;

    size_t len = strlen(new_node->data);
    pthread_mutex_unlock(&q->mutex);
    sem_post(&q->full);

    return len;
}

int mymsgget(queue *q, char *buf, size_t bufsize) {
    if (q == NULL || buf == NULL || bufsize == 0) return 0;

    if (sem_wait(&q->full) != 0) return 0;

    pthread_mutex_lock(&q->mutex);
    if (q->dropped) {
        pthread_mutex_unlock(&q->mutex);
        sem_post(&q->full);
        return 0;
    }

    if (q->head == NULL) {
        pthread_mutex_unlock(&q->mutex);
        sem_post(&q->full);
        return 0;
    }

    msg_node *node = q->head;
    strncpy(buf, node->data, bufsize - 1);
    buf[bufsize - 1] = '\0';

    q->head = node->next;
    if (q->head == NULL) {
        q->tail = NULL;
    }
    q->count--;

    size_t len = strlen(buf);
    free(node);

    pthread_mutex_unlock(&q->mutex);
    sem_post(&q->empty);

    return len;
}