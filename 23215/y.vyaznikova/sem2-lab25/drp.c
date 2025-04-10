#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "queue.h"

queue msg_queue;

void* producer(void* arg) {
    long producer_id = (long)arg;
    char buff[MAX_MSG_LENGTH + 1];
    sprintf(buff, "Producer %lu: Initial message", producer_id);
    int putted = mymsgput(&msg_queue, buff);
    if (putted <= 0) {
        printf("Producer %lu: mymsgput returned %d after drop (expected 0)\n", producer_id, putted);
    }
    printf("Producer %lu: Exiting after put.\n", producer_id);
    return NULL;
}

void* consumer(void* arg) {
    long consumer_id = (long)arg;
    char buff[MAX_MSG_LENGTH + 1];
    int count = mymsgget(&msg_queue, buff, sizeof(buff));
    if (count <= 0) {
        printf("Consumer %lu: mymsgget returned %d after drop (expected 0)\n", consumer_id, count);
    }
    printf("Consumer %lu: Exiting after get.\n", consumer_id);
    return NULL;
}

int main() {
    mymsginit(&msg_queue);

    pthread_t prod1, cons1;

    pthread_create(&prod1, NULL, producer, (void*) 1);
    pthread_create(&cons1, NULL, consumer, (void*) 1);

    sleep(1);

    printf("Main: Calling mymsgdrop.\n");
    mymsgdrop(&msg_queue);

    pthread_join(prod1, NULL);
    pthread_join(cons1, NULL);

    mymsgdestroy(&msg_queue);
    printf("Main: Exiting.\n");
    return 0;
}