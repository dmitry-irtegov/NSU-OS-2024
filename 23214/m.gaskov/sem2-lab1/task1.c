#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


void *thread_function(void *arg) {
    for (int i = 0; i < 10; i++) {
        printf("Child thread: Line %d\n", i + 1);
    }
    return NULL;
}

int main() {
    pthread_t thread;
    int code;
    if ((code = pthread_create(&thread, NULL, thread_function, NULL)) != 0) {
        fprintf(stderr, "Failed to create thread: %d.\n", code);
        return EXIT_FAILURE;
    }
    for (int i = 0; i < 10; i++) {
        printf("Parent thread: Line %d\n", i + 1);
    }
    pthread_exit((void *)EXIT_SUCCESS);
}

