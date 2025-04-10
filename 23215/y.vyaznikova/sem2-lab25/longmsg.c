#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "queue.h"

void *producer(void *arg) {
    queue *q = (queue *)arg;

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

    for (int i = 0; i < 15; i++) {
        char buffer[MAX_MSG_LENGTH + 1];
        memset(buffer, 0, sizeof(buffer));
        strncpy(buffer, messages[i], MAX_MSG_LENGTH);
        buffer[MAX_MSG_LENGTH] = '\0';

        int len = mymsgput(q, buffer);
        printf("Producer %lu put message (original length: %lu, sent length: %d): %.80s%s\n",
               (unsigned long)pthread_self(),
               strlen(messages[i]),
               len,
               messages[i],
               strlen(messages[i]) > MAX_MSG_LENGTH ? " [TRUNCATED]" : "");
        sleep(1);
    }
    return NULL;
}

void *consumer(void *arg) {
    queue *q = (queue *)arg;
    char buf[MAX_MSG_LENGTH + 1];
    for (int i = 0; i < 15; i++) {
        int len = mymsgget(q, buf, sizeof(buf));
        if (len > 0) {
            printf("Consumer %lu got message: %s\n", (unsigned long)pthread_self(), buf);
        }
        sleep(1);
    }
    return NULL;
}

int main() {
    queue q;
    mymsginit(&q);

    pthread_t prod1, prod2, cons1, cons2;

    pthread_create(&prod1, NULL, producer, &q);
    pthread_create(&prod2, NULL, producer, &q);
    pthread_create(&cons1, NULL, consumer, &q);
    pthread_create(&cons2, NULL, consumer, &q);

    pthread_join(prod1, NULL);
    pthread_join(prod2, NULL);
    pthread_join(cons1, NULL);
    pthread_join(cons2, NULL);

    mymsgdestroy(&q);
    return 0;
}