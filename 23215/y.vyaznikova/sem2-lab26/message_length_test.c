#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include "task25_cond.h"

const char *messages[] = {
    "Short message.",
    "This is a medium length message for testing purposes.",
    "This message is intentionally made very very long to exceed the eighty characters limit that the queue enforces strictly to prevent buffer overflows and long message corruption issues.",
    "Another short one.",
    "This is a long message that will also be cut at eighty characters. Let's see how that goes.",
    "This is just a message.",
    "This is merely a message.",
    "This is simply a message.",
    "This is plainly a message.",
    "This is purely a message.",
    "This is only a message.",
    "This is but a message.",
    "This is solely a message.",
    "This is strictly a message.",
    "This is truly a message."
};

void *message_producer(void *arg) {
    queue *q = (queue *)arg;
    for (int i = 0; i < sizeof(messages)/sizeof(messages[0]); i++) {
        int len = mymsgput(q, (char*)messages[i]);
        printf("\n[Producer] Trying to put message %d:\n", i + 1);
        printf("Original message (%zu chars): %s\n", strlen(messages[i]), messages[i]);
        if (len > 0) {
            printf("Successfully put message (stored %d chars)\n", len);
            if (len < strlen(messages[i])) {
                printf("Message was truncated to %d characters\n", len);
            }
        } else {
            printf("Failed to put message (queue dropped)\n");
            break;
        }
        sleep(1);
    }
    return NULL;
}

void *message_consumer(void *arg) {
    queue *q = (queue *)arg;
    char buf[MAX_MSG_LENGTH + 1];
    int msg_count = 0;
    
    while (1) {
        int len = mymsgget(q, buf, sizeof(buf));
        if (len > 0) {
            printf("\n[Consumer] Got message %d:\n", ++msg_count);
            printf("Received message (%d chars): %s\n", len, buf);
        } else {
            printf("\n[Consumer] Queue dropped or empty\n");
            break;
        }
        usleep(500000);
    }
    return NULL;
}

int main() {
    printf("=== Message Length Test ===\n");
    printf("This test verifies correct handling of messages with different lengths\n");
    printf("Maximum message length should be %d characters\n\n", MAX_MSG_LENGTH);

    queue q1;
    mymsginit(&q1);
    
    pthread_t prod1, cons1;
    pthread_create(&prod1, NULL, message_producer, &q1);
    pthread_create(&cons1, NULL, message_consumer, &q1);
    
    sleep(5);
    
    printf("\nDropping first queue to test message handling after drop...\n");
    mymsqdrop(&q1);
    
    pthread_join(prod1, NULL);
    pthread_join(cons1, NULL);
    mymsgdestroy(&q1);
    
    printf("\nFirst queue destroyed. Creating new queue for second part of test...\n\n");
    sleep(2);

    queue q2;
    mymsginit(&q2);
    
    pthread_t prod2, cons2;
    pthread_create(&prod2, NULL, message_producer, &q2);
    pthread_create(&cons2, NULL, message_consumer, &q2);
    
    sleep(10);
    
    printf("\nTest completed, destroying second queue...\n");
    mymsgdestroy(&q2);

    return 0;
}