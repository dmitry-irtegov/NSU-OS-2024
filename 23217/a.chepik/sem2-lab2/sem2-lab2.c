#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void* thread_function(void* arg) {
    for (int i = 0; i < 10; i++) {
        printf("Child.\n");
    }

    return NULL;
}

int main() {
    pthread_t thread;
    int pc_code = pthread_create(&thread, NULL, thread_function, NULL);

    if (pc_code != 0) {
        printf("pthread_create() returned a non-zero value.\n");
        exit(-1);
    }

    pthread_join(thread, NULL);

    for (int i = 0; i < 10; i++) {
        printf("Parent.\n");
    }

    pthread_exit(NULL);
}
