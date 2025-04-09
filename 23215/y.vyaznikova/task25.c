#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <unistd.h>

#define MAX_QUEUE_SIZE 10
#define MAX_MSG_LENGTH 80

typedef struct msg_node {
    char data[MAX_MSG_LENGTH + 1];
    struct msg_node *next;
} msg_node;

typedef struct {
    msg_node *head;
    msg_node *tail;
    int count;
    sem_t empty;
    sem_t full;
    pthread_mutex_t mutex;
    int dropped;
} queue;

void mymsginit(queue *q) {
    if (q == NULL) {
        return;
    }

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
    if (q == NULL) {
        return;
    }

    pthread_mutex_lock(&q->mutex);
    msg_node *current = q->head;
    while (current != NULL) {
        msg_node *temp = current;
        current = current->next;
        free(temp);
    }
    q->head = NULL;
    q->tail = NULL;
    pthread_mutex_unlock(&q->mutex);

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

    while (q->count < MAX_QUEUE_SIZE) {
        sem_post(&q->full);
        q->count++;
    }
    while (q->count > 0) {
        sem_post(&q->empty);
        q->count--;
    }

    msg_node *current = q->head;
    while (current != NULL) {
        msg_node *temp = current;
        current = current->next;
        free(temp);
    }
    q->head = NULL;
    q->tail = NULL;

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
    pthread_mutex_unlock(&q->mutex);

    if (sem_wait(&q->empty) != 0) {
        if (errno == EINTR) return 0;
    }

    pthread_mutex_lock(&q->mutex);
    if (q->dropped) {
        sem_post(&q->empty);
        pthread_mutex_unlock(&q->mutex);
        return 0;
    }

    msg_node *new_node = (msg_node *)malloc(sizeof(msg_node));
    if (new_node == NULL) {
        sem_post(&q->empty);
        pthread_mutex_unlock(&q->mutex);
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
    if (q == NULL || buf == NULL || bufsize == 0) {
        return 0;
    }

    pthread_mutex_lock(&q->mutex);
    if (q->dropped) {
        pthread_mutex_unlock(&q->mutex);
        return 0;
    }
    pthread_mutex_unlock(&q->mutex);

    if (sem_wait(&q->full) != 0) {
        if (errno == EINTR) return 0;
    }

    pthread_mutex_lock(&q->mutex);
    if (q->dropped) {
        sem_post(&q->full);
        pthread_mutex_unlock(&q->mutex);
        return 0;
    }

    if (q->head == NULL) {
        sem_post(&q->full);
        pthread_mutex_unlock(&q->mutex);
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

void *producer(void *arg) {
    queue *q = (queue *)arg;

    const char *messages[] = {
        "Short message.",
        "This is a medium length message for testing purposes.",
        "This message is intentionally made very very long to exceed the eighty characters limit that the queue enforces strictly to prevent buffer overflows and long message corruption issues.",
        "Another short one.",
        "This is a long message that will also be cut at eighty characters. Let's see how that goes.",
        "This is just a message.",
        "This is merely a message.",
        "This is simply a message.",
        "This is plainly a message.",
        "This is purely a message.",
        "This is only a message.",
        "This is but a message.",
        "This is solely a message.",
        "This is strictly a message.",
        "This is truly a message."
    };

    for (int i = 0; i < 15; i++) {
        char buffer[MAX_MSG_LENGTH + 1];
        memset(buffer, 0, sizeof(buffer));
        strncpy(buffer, messages[i], MAX_MSG_LENGTH);
        buffer[MAX_MSG_LENGTH] = '\0';

        int len = mymsgput(q, buffer);
        printf("Producer %lu put message (original length: %lu, sent length: %d): %.80s%s\n",
               (unsigned long)pthread_self(),
               strlen(messages[i]),
               len,
               messages[i],
               strlen(messages[i]) > MAX_MSG_LENGTH ? " [TRUNCATED]" : "");
        sleep(1);
    }
    return NULL;
}

void *consumer(void *arg) {
    queue *q = (queue *)arg;
    char buf[MAX_MSG_LENGTH + 1];
    for (int i = 0; i < 15; i++) {
        int len = mymsgget(q, buf, sizeof(buf));
        if (len > 0) {
            printf("Consumer %lu got message: %s\n", (unsigned long)pthread_self(), buf);
        }
        sleep(1);
    }
    return NULL;
}

int main() {
    queue q;
    mymsginit(&q);

    pthread_t prod1, prod2, cons1, cons2;

    pthread_create(&prod1, NULL, producer, &q);
    pthread_create(&prod2, NULL, producer, &q);
    pthread_create(&cons1, NULL, consumer, &q);
    pthread_create(&cons2, NULL, consumer, &q);

    pthread_join(prod1, NULL);
    pthread_join(prod2, NULL);
    pthread_join(cons1, NULL);
    pthread_join(cons2, NULL);

    mymsgdestroy(&q);
    return 0;
}