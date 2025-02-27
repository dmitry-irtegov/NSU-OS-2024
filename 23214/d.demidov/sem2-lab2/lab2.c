#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

void *thread_body(void *param) {
    for (int i = 0; i < 10; i++) {
        printf("Child: Line %d\n", i);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    pthread_t thread;
    if (pthread_create(&thread, NULL, thread_body, NULL) != 0) {
        perror("Error creating thread");
        exit(EXIT_FAILURE);
    }

    if (0 != pthread_join(thread, NULL)) {
        perror("Error joining thread");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < 10; i++) {
        printf("Parent: Line %d\n", i);
    }

    return 0;
}
