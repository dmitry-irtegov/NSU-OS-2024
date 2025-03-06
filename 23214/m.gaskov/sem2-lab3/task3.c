#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void *thread_function(void *arg) {
    char **messages = (char **)arg;
    for (int i = 0; messages[i] != NULL; i++) {
        printf("%s\n", messages[i]);
    }
    return NULL;
}

int main() {
    pthread_t threads[4];

    char *messages1[] = {"Thread 1: Line 1", "Thread 1: Line 2", "Thread 1: Line 3", NULL};
    char *messages2[] = {"Thread 2: Line 1", "Thread 2: Line 2", NULL};
    char *messages3[] = {"Thread 3: Line 1", "Thread 3: Line 2", "Thread 3: Line 3", "Thread 3: Line 4", NULL};
    char *messages4[] = {"Thread 4: Line 1", NULL};

    char **thread_data[4] = {messages1, messages2, messages3, messages4};

    int code;
    for (int i = 0; i < 4; i++) {
        if ((code = pthread_create(&threads[i], NULL, thread_function, thread_data[i])) != 0) {
            fprintf(stderr, "Failed to create thread %d: %d\n", i + 1, code);
            return EXIT_FAILURE;
        }
    }

    for (int i = 0; i < 4; i++) {
        if ((code = pthread_join(threads[i], NULL)) != 0) {
            fprintf(stderr, "Failed to join thread %d: %d\n", i + 1, code);
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

