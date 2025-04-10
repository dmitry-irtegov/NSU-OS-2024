#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "queue.h"

queue mqueue;

void* producer(void* arg) {
    long producer_id = (long)arg;
    int putted = 1;
    char buff[MAX_MSG_LENGTH + 1];
    int msg_num = 0;
    while (putted > 0) {
        sprintf(buff, "Producer %lu: Message %d", producer_id, msg_num++);
        putted = mymsgput(&mqueue, buff);
        if (putted <= 0) {
            printf("Producer %lu: Failed to put message or queue dropped.\n", producer_id);
        }
    }
    printf("Producer %lu: Exiting.\n", producer_id);
    return NULL;
}

void* slow_consumer(void* arg) {
    long consumer_id = (long)arg;
    int count;
    char buff[MAX_MSG_LENGTH + 1];
    while ((count = mymsgget(&mqueue, buff, sizeof(buff))) > 0) {
        buff[count] = 0;
        printf("Consumer %lu: Received: \"%s\"\n", consumer_id, buff);
        usleep(700007);
    }
    printf("Consumer %lu: Exiting.\n", consumer_id);
    return NULL;
}

int main() {
    mymsginit(&mqueue);
    
    pthread_t prod1, prod2, cons1, cons2;

    pthread_create(&prod1, NULL, producer, (void*) 1);
    pthread_create(&prod2, NULL, producer, (void*) 2);
    pthread_create(&cons1, NULL, slow_consumer, (void*) 1);
    pthread_create(&cons2, NULL, slow_consumer, (void*) 2);

    pthread_join(prod1, NULL);
    pthread_join(prod2, NULL);
    pthread_join(cons1, NULL);
    pthread_join(cons2, NULL);

    mymsgdestroy(&mqueue);    
    printf("Main: Exiting.\n");
    return 0;
}