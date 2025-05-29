#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <signal.h>
#include <alloca.h>
#include "task25_sem.h"

void *producer(void *arg) {
    queue *q = (queue *)arg;
    static int producer_id = 0;
    int my_id = ++producer_id;
    
    for (int i = 0; i < 10; i++) {
        char msg[MAX_MSG_LENGTH + 1];
        snprintf(msg, sizeof(msg), "Producer %d message %d", my_id, i);
        int len = mymsgput(q, msg);
        if (len > 0) {
            printf("[Producer %d] Put message %d, length: %d\n", my_id, i, len);
            usleep(500000);
        } else {
            printf("[Producer %d] Failed to put message %d (queue dropped)\n", my_id, i);
            break;
        }
    }
    return NULL;
}

void *consumer(void *arg) {
    queue *q = (queue *)arg;
    static int consumer_id = 0;
    int my_id = ++consumer_id;
    char buf[MAX_MSG_LENGTH + 1];
    
    while (1) {
        int len = mymsgget(q, buf, sizeof(buf));
        if (len > 0) {
            printf("[Consumer %d] Got message: %s, length: %d\n", my_id, buf, len);
            usleep(750000);
        } else {
            printf("[Consumer %d] Queue dropped or empty\n", my_id);
            break;
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s nproducers nconsumers\n", argv[0]);
        return 1;
    }

    long nproducers = atol(argv[1]);
    long nconsumers = atol(argv[2]);

    if (nproducers == 0 || nconsumers == 0) {
        fprintf(stderr, "Usage: %s nproducers nconsumers\n", argv[0]);
        return 1;
    }

    pthread_t *producers = alloca(sizeof(pthread_t) * nproducers);
    pthread_t *consumers = alloca(sizeof(pthread_t) * nconsumers);

    queue q;
    mymsginit(&q);

    printf("=== Flexible Test with %ld producers and %ld consumers ===\n\n", 
           nproducers, nconsumers);


    for (long i = 0; i < nproducers || i < nconsumers; i++) {
        if (i < nproducers) {
            pthread_create(&producers[i], NULL, producer, &q);
        }
        if (i < nconsumers) {
            pthread_create(&consumers[i], NULL, consumer, &q);
        }
    }

    sleep(10);
    printf("\nDropping queue...\n");
    mymsqdrop(&q);


    for (long i = 0; i < nproducers || i < nconsumers; i++) {
        if (i < nproducers) {
            pthread_join(producers[i], NULL);
        }
        if (i < nconsumers) {
            pthread_join(consumers[i], NULL);
        }
    }

    mymsgdestroy(&q);
    printf("All threads quit and queue destroyed\n");

    return 0;
}
