#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include "task25.h"

void *fast_producer(void *arg) {
    queue *q = (queue *)arg;
    static int producer_id = 0;
    int my_id = ++producer_id;
    
    for (int i = 0; i < 10; i++) {
        char msg[MAX_MSG_LENGTH + 1];
        snprintf(msg, sizeof(msg), "Fast producer %d message %d", my_id, i);
        int len = mymsgput(q, msg);
        if (len > 0) {
            printf("[Fast Producer %d] Put message %d, length: %d\n", my_id, i, len);
            usleep(100000);
        } else {
            printf("[Fast Producer %d] Failed to put message %d (queue dropped)\n", my_id, i);
            break;
        }
    }
    return NULL;
}

void *slow_consumer(void *arg) {
    queue *q = (queue *)arg;
    static int consumer_id = 0;
    int my_id = ++consumer_id;
    char buf[MAX_MSG_LENGTH + 1];
    int msg_count = 0;
    
    while (1) {
        int len = mymsgget(q, buf, sizeof(buf));
        if (len > 0) {
            printf("[Slow Consumer %d] Got message: %s, length: %d\n", my_id, buf, len);
            msg_count++;
            sleep(1);
        } else {
            printf("[Slow Consumer %d] Queue dropped or empty\n", my_id);
            break;
        }
    }
    return NULL;
}

int main() {
    printf("=== Multiple Fast Producers Test ===\n\n");

    queue q1;
    mymsginit(&q1);
    
    pthread_t prod1, prod2, cons1, cons2;
    

    pthread_create(&prod1, NULL, fast_producer, &q1);
    pthread_create(&prod2, NULL, fast_producer, &q1);
    pthread_create(&cons1, NULL, slow_consumer, &q1);
    pthread_create(&cons2, NULL, slow_consumer, &q1);
    
    sleep(10);
    printf("\nDropping first queue...\n");
    mymsqdrop(&q1);
    
    pthread_join(prod1, NULL);
    pthread_join(prod2, NULL);
    pthread_join(cons1, NULL);
    pthread_join(cons2, NULL);
    mymsgdestroy(&q1);
    
    printf("\nFirst queue destroyed. Creating new queue...\n\n");
    sleep(2);

    queue q2;
    mymsginit(&q2);
    

    pthread_t new_prod1, new_prod2, new_cons1, new_cons2;
    pthread_create(&new_prod1, NULL, fast_producer, &q2);
    pthread_create(&new_prod2, NULL, fast_producer, &q2);
    pthread_create(&new_cons1, NULL, slow_consumer, &q2);
    pthread_create(&new_cons2, NULL, slow_consumer, &q2);
    
    sleep(15);
    printf("\nTest completed, destroying second queue...\n");
    
    pthread_join(new_prod1, NULL);
    pthread_join(new_prod2, NULL);
    pthread_join(new_cons1, NULL);
    pthread_join(new_cons2, NULL);
    mymsgdestroy(&q2);

    return 0;
}
