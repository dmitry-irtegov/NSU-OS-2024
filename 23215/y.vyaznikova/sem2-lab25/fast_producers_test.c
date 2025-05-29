#include <assert.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#if defined(USE_SEMAPHORES)
#include "task25_sem.h"
#else
#include "task25_cond.h"
#endif

void my_assert(int expr) {
    assert(expr);
}

queue q;

void* producer(void* arg) {
    int putted;
    char buff[100];
    int msg_num = 0;
    do {
        sprintf(buff, "producer %lu produced %d", (unsigned long) arg, msg_num++);
        printf("producer %lu sent message %d\n", (unsigned long) arg, msg_num - 1);
        putted = mymsgput(&q, buff);
    } while (putted > 0);
    return NULL;
}

void* slow_consumer(void* arg) {
    int count;
    char buff[100];
    while ((count = mymsgget(&q, buff, 100)) > 0) { 
        buff[count] = 0;
        printf("consumer %lu has received \"%s\"\n", (unsigned long) arg, buff);
        usleep(500000);
    };
    return NULL;
}

int main() {
    mymsginit(&q);
    
    pthread_t prod1, prod2, cons1, cons2;

    my_assert(0 == pthread_create(&prod1, NULL, producer, (void*) 1));
    my_assert(0 == pthread_create(&prod2, NULL, producer, (void*) 2));
    my_assert(0 == pthread_create(&cons1, NULL, slow_consumer, (void*) 1));
    my_assert(0 == pthread_create(&cons2, NULL, slow_consumer, (void*) 2));
    
    sleep(10);

    mymsqdrop(&q);

    pthread_join(prod1, NULL);
    pthread_join(prod2, NULL);
    pthread_join(cons1, NULL);
    pthread_join(cons2, NULL);

    mymsgdestroy(&q);    
    return 0;
}