#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

void* func(void* arg) {
    for (int i = 0; i < 10; i++) {
        printf("New thread: string of text\n");
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    pthread_t thread;

    int res = pthread_create(&thread, NULL, func, NULL);
    if (res != 0) {
        fprintf(stderr, "%s: creating thread: %s\n", argv[0], strerror(res));
        exit(1);
    }


    for (int i = 0; i < 10; i++) {
        printf("Main thread: string of text\n");
    }

    pthread_exit(NULL);
}

