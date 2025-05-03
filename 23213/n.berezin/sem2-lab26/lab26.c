#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <alloca.h>
#include <unistd.h>
#include <inttypes.h>
#include "mymsg.h"

#define PRODUCER_CONSUMER_AMOUNT 2

void *producer(void *pq) {
    queue *q = (queue*)pq;

    for(int i = 0; i < 1000; i++) {
        char buf[40];

        snprintf(buf, 40, "Message %d from thread %ju", i, (uintmax_t)pthread_self());
        if (!mymsgput(q, buf)){
            return NULL;
        }
    }

    return NULL;
}

void *consumer(void *pq) {
    queue *q = (queue *)pq;

    while (1) {
        char buf[41];
        int msglen = mymsgget(q, buf, sizeof(buf));

        if (msglen > 0) {
            printf("Received by thread %ju: %.*s\n", (uintmax_t)pthread_self(), msglen, buf);
        } else {
            printf("Recieved empty message by thread %ju\n", (uintmax_t)pthread_self());
            return NULL;
        }
    }
}

int main(int argc, char **argv) {
    pthread_t producers[PRODUCER_CONSUMER_AMOUNT], consumers[PRODUCER_CONSUMER_AMOUNT];

    queue q;
    mymsginit(&q);

    for(int i = 0; i < PRODUCER_CONSUMER_AMOUNT; i++) {
        int prod_err = pthread_create(&producers[i], NULL, producer, &q);
        int cons_err = pthread_create(&consumers[i], NULL, consumer, &q);

        if (prod_err || cons_err) {
            fprintf(stderr, "Failed to create thread\n");
            exit(1);
        }
    }

    sleep(1);

    mymsgdrop(&q);

    for(int i = 0; i < PRODUCER_CONSUMER_AMOUNT; i++) {
        int prod_err = pthread_join(producers[i], NULL);
        int cons_err = pthread_join(consumers[i], NULL);

        if (prod_err || cons_err) {
            fprintf(stderr, "Failed to join thread\n");
            exit(1);
        }
    }

    mymsgdestroy(&q);
    printf("All threads quit and queue destroyed\n");

    return 0;
}
