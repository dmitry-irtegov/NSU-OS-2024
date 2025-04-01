#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

void* thread_func(void* param) {
    for (int i = 0; i < 10; i++)
        printf("Child\n");
    return NULL;
}

int main(int argc, char* argv[]) {
    pthread_t thread;

    int err = pthread_create(&thread, NULL, thread_func, NULL);
    if (err != 0) {
        fprintf(stderr, "Error while creating: %s\n", strerror(err));
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < 10; i++)
        printf("Parent\n");

    pthread_exit(NULL);
}
