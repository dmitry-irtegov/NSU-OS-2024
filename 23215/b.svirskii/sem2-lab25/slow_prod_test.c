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

void* slow_producer(void* arg) {
    int putted;
    do {
        putted = mymsgput(&queue, "hello!");
        printf("slow producer putted msg\n");
        sleep(1);
    } while (putted > 0);
    return NULL;
}

void* consumer(void* arg) {
    int count;
    char buff[10];
    do {
        count = mymsgget(&queue, buff, 10);
        buff[count] = 0;
        printf("consumer has received \"%s\"\n", buff);
    } while (count > 0);
    return NULL;
}



int main() {
    mymsginit(&queue);
    
    pthread_t prod, cons;

    my_assert(0 == pthread_create(&prod, NULL, slow_producer, NULL));
    my_assert(0 == pthread_create(&cons, NULL, consumer, NULL));
    
    sleep(10);

    mymsgdrop(&queue);

    pthread_join(prod, NULL);
    pthread_join(cons, NULL);

    mymsgdestroy(&queue);    
    return 0;
}
