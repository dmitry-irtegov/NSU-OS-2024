#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

void* thread_function(void* arg) {
    for (int i = 0; i < 10; i++) {
        printf("Child thread: line %d\n", i);
    }
    return NULL;
}

int main() {
    pthread_t thread;
    int result = pthread_create(&thread, NULL, thread_function, NULL);
    if (result != 0) {
        perror("Error creating thread");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < 10; i++) {
        printf("Parent thread: line %d\n", i);
    }
    pthread_exit(NULL);
}
