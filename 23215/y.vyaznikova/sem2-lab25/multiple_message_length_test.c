#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include "task25.h"

typedef struct {
    queue *q;
    int msg_length;
    int producer_id;
} producer_args;

void *length_producer(void *arg) {
    producer_args *args = (producer_args *)arg;
    queue *q = args->q;
    int msg_length = args->msg_length;
    int my_id = args->producer_id;
    
    for (int i = 0; i < 5; i++) {
        char msg[MAX_MSG_LENGTH + 1];
        memset(msg, 'A' + my_id, msg_length);
        msg[msg_length] = '\0';
        
        char numbered_msg[MAX_MSG_LENGTH + 1];
        snprintf(numbered_msg, sizeof(numbered_msg), "P%d-M%d-%s", my_id, i, msg);
        
        int len = mymsgput(q, numbered_msg);
        if (len > 0) {
            printf("[Producer %d] Put message %d, requested length: %d, actual length: %d\n", 
                   my_id, i, msg_length, len);
            usleep(500000);
        } else {
            printf("[Producer %d] Failed to put message %d (queue dropped)\n", my_id, i);
            break;
        }
    }
    free(arg);
    return NULL;
}

void *length_consumer(void *arg) {
    queue *q = (queue *)arg;
    static int consumer_id = 0;
    int my_id = ++consumer_id;
    char buf[MAX_MSG_LENGTH + 1];
    
    while (1) {
        int len = mymsgget(q, buf, sizeof(buf));
        if (len > 0) {
            printf("[Consumer %d] Got message: %s, length: %d\n", my_id, buf, len);
            usleep(750000);
        } else {
            printf("[Consumer %d] Queue dropped or empty\n", my_id);
            break;
        }
    }
    return NULL;
}

void test_multiple_lengths(queue *q, int lengths[], int num_lengths) {
    pthread_t prod1, prod2, cons1, cons2;
    
    producer_args *args1 = malloc(sizeof(producer_args));
    args1->q = q;
    args1->msg_length = lengths[0];
    args1->producer_id = 1;
    
    producer_args *args2 = malloc(sizeof(producer_args));
    args2->q = q;
    args2->msg_length = lengths[1];
    args2->producer_id = 2;
    
    pthread_create(&prod1, NULL, length_producer, args1);
    pthread_create(&prod2, NULL, length_producer, args2);
    pthread_create(&cons1, NULL, length_consumer, q);
    pthread_create(&cons2, NULL, length_consumer, q);
    
    pthread_join(prod1, NULL);
    pthread_join(prod2, NULL);
    pthread_join(cons1, NULL);
    pthread_join(cons2, NULL);
}

int main() {
    printf("=== Multiple Message Length Test ===\n\n");

    int test_lengths[][2] = {
        {20, 40},
        {79, 80},
        {90, 100},
        {5, 10},
    };
    
    for (int i = 0; i < 4; i++) {
        printf("\nTest case %d: Producer 1 length = %d, Producer 2 length = %d\n",
               i + 1, test_lengths[i][0], test_lengths[i][1]);
        
        queue q;
        mymsginit(&q);
        
        if (i == 1) {
            pthread_t dropper;
            pthread_create(&dropper, NULL, (void *(*)(void *))mymsqdrop, &q);
            pthread_detach(dropper);
        }
        
        test_multiple_lengths(&q, test_lengths[i], 2);
        mymsgdestroy(&q);
        
        printf("\nTest case %d completed\n", i + 1);
        sleep(2);
    }
    
    printf("\nAll message length tests completed\n");
    return 0;
}