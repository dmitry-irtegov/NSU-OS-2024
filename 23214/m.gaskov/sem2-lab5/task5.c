#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void cleanup_function(void *arg) {
    printf("Hello from cleanup function!\n");
}

void *thread_function(void *arg) {
    int i = 0;
    pthread_cleanup_push(cleanup_function, NULL);
    while (1) {
        printf("hi %d\n", i++);
        usleep(100000);
    }
    pthread_cleanup_pop(0);
    return NULL;
}

int main() {
    pthread_t thread;
    int code;

    if ((code = pthread_create(&thread, NULL, thread_function, NULL)) != 0) {
        fprintf(stderr, "Failed to create thread: %d\n", code);
        return EXIT_FAILURE;
    }

    sleep(2);

    if ((code = pthread_cancel(thread)) != 0) {
        fprintf(stderr, "Failed to cancel thread: %d\n", code);
        return EXIT_FAILURE;
    }

    if ((code = pthread_join(thread, NULL)) != 0) {
        fprintf(stderr, "Failed to join thread: %d\n", code);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
