#include <pthread.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <semaphore.h>
#include "msgqueue_sem.h"

void *producer(void *pq) {
    queue *q = (queue *)pq;

    int i = 0;
    for (i = 0; i < 1000; i++) {
        char buf[40];
        sprintf(buf, "Message %d from thread %lu", i, pthread_self());
        if (!mymsgput(q, buf)) 
            return NULL;
    }
    return NULL;
}

void *consumer(void *pq) {
    queue *q = (queue *)pq;
    int i = 0;
    do {
        char buf[41];
        i = mymsgget(q, buf, sizeof(buf));
        if (i == 0) 
            return NULL;
        else {
            assert((strstr(buf, "Message") != NULL) && (strstr(buf, "from thread") != NULL));
            printf("Received by thread %lu: %s\n", pthread_self(), buf);
        }
    } while (1);
    return NULL;
}

int main(int argc, char **argv) {
    int nproducers, nconsumers;
    queue q;
    int i;
    if (argc < 3) {
        fprintf(stderr, "Usage: %s nproducers nconsumers\n", argv[0]);
        return 0;
    }
    nproducers = atoi(argv[1]);
    nconsumers = atoi(argv[2]);

    if (nproducers == 0 || nconsumers == 0) {
        fprintf(stderr, "Usage: %s nproducers nconsumers\n", argv[0]);
        return 0;
    }

    pthread_t producers[nproducers];
    pthread_t consumers[nconsumers];
    mymsginit(&q);
    int code;
    for (int i = 0; i < nproducers; i++) {
        code = pthread_create(&producers[i], NULL, producer, &q);
        if (code != 0) {
            fprintf(stderr, "pthread_create_error %s\n", strerror(code));
            exit(1);
        }
    }
    for (int j = 0; j < nconsumers; j++) {
        code = pthread_create(&consumers[j], NULL, consumer, &q);
        if (code != 0) {
            fprintf(stderr, "pthread_create_error %s\n", strerror(code));
            exit(1);
        }
    }

    sleep(10);

    mymsgdrop(&q);

    for (int i = 0; i < nproducers; i++) {
        pthread_join(producers[i], NULL);
    }
    for (int j = 0; j < nconsumers; j++) {
        pthread_join(consumers[j], NULL);
    }
    mymsgdestroy(&q);
    printf("All threads quit and queue destroyed\n");
    return 0;
}
