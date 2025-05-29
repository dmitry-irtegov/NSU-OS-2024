#include <assert.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "queue.h"

void my_assert(int expr) { assert(expr); }

Queue queue;

void* producer(void* arg) {
    int putted;
    char buff[100];
    int msg_num = 0;
    do {
        sprintf(buff, "producer %lu produced %d", (unsigned long)arg,
                msg_num++);

        putted = mymsgput(&queue, buff);
        printf("%s\n", buff);
        usleep(500000);
    } while (putted > 0);
    return NULL;
}

void* consumer(void* arg) {
    int count;
    char buff[100];
    while ((count = mymsgget(&queue, buff, 100)) > 0) {
        buff[count] = 0;
        printf("consumer %lu has received \"%s\"\n", (unsigned long)arg, buff);
    };
    return NULL;
}

int main() {
    mymsginit(&queue);

    pthread_t prod1, prod2, cons1, cons2;

    my_assert(0 == pthread_create(&prod1, NULL, producer, (void*)1));
    my_assert(0 == pthread_create(&prod2, NULL, producer, (void*)2));
    my_assert(0 == pthread_create(&cons1, NULL, consumer, (void*)1));
    my_assert(0 == pthread_create(&cons2, NULL, consumer, (void*)2));

    sleep(10);

    mymsgdrop(&queue);

    pthread_join(prod1, NULL);
    pthread_join(prod2, NULL);
    pthread_join(cons1, NULL);
    pthread_join(cons2, NULL);

    mymsgdestroy(&queue);
    return 0;
}
