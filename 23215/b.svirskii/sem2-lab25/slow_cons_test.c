#include <assert.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "queue.h"

void my_assert(int expr) {
    assert(expr);
}

Queue queue;

void* producer(void* arg) {
    int putted;
    do {
        putted = mymsgput(&queue, "hello!");
        printf("producer putted msg\n");
    } while (putted > 0);
    return NULL;
}

void* slow_consumer(void* arg) {
    int count;
    char buff[10];
    do {
        count = mymsgget(&queue, buff, 10);
        buff[count] = 0;
        printf("slow consumer has received \"%s\"\n", buff);
        sleep(1);
    } while (count > 0);
    return NULL;
}



int main() {
    mymsginit(&queue);
    
    pthread_t prod, cons;

    my_assert(0 == pthread_create(&prod, NULL, producer, NULL));
    my_assert(0 == pthread_create(&cons, NULL, slow_consumer, NULL));
    
    sleep(10);

    mymsgdrop(&queue);

    pthread_join(prod, NULL);
    pthread_join(cons, NULL);

    mymsgdestroy(&queue);    
    return 0;
}
