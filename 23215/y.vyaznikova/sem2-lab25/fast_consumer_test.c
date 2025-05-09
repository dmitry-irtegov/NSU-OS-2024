#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include "task25.h"

void *slow_producer(void *arg) {
    queue *q = (queue *)arg;
    for (int i = 0; i < 20; i++) {
        char msg[MAX_MSG_LENGTH + 1];
        snprintf(msg, sizeof(msg), "Slow producer message %d", i);
        int len = mymsgput(q, msg);
        if (len > 0) {
            printf("[Slow Producer] Put message %d, length: %d\n", i, len);
            sleep(1);
        } else {
            printf("[Slow Producer] Failed to put message %d (queue dropped)\n", i);
            break;
        }
    }
    return NULL;
}

void *fast_consumer(void *arg) {
    queue *q = (queue *)arg;
    char buf[MAX_MSG_LENGTH + 1];
    int msg_count = 0;
    while (1) {
        int len = mymsgget(q, buf, sizeof(buf));
        if (len > 0) {
            printf("[Fast Consumer] Got message: %s, length: %d\n", buf, len);
            msg_count++;
        } else {
            printf("[Fast Consumer] Queue dropped or empty\n");
            break;
        }
    }
    return NULL;
}

int main() {
    printf("=== Fast Consumer Test ===\n\n");

    queue q1;
    mymsginit(&q1);
    
    pthread_t prod1, cons1;
    pthread_create(&prod1, NULL, slow_producer, &q1);
    pthread_create(&cons1, NULL, fast_consumer, &q1);
    
    sleep(5);
    printf("\nDropping first queue...\n");
    mymsqdrop(&q1);
    
    pthread_join(prod1, NULL);
    pthread_join(cons1, NULL);
    mymsgdestroy(&q1);
    
    printf("\nFirst queue destroyed. Creating new queue...\n\n");
    sleep(2);

    queue q2;
    mymsginit(&q2);
    
    pthread_t prod2, cons2;
    pthread_create(&prod2, NULL, slow_producer, &q2);
    pthread_create(&cons2, NULL, fast_consumer, &q2);
    
    sleep(10);
    printf("\nTest completed, destroying second queue...\n");
    mymsgdestroy(&q2);

    return 0;
}