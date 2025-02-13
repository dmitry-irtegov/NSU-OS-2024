#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

void* thread_function(void* arg) {
    for (int i = 0; i < 10; i++) {
        printf("Child %d\n", i + 1);
    }
    return NULL;
}

int main() {
    pthread_t thread;
    if (pthread_create(&thread, NULL, thread_function, NULL) != 0) {
        perror("create error");
        exit(1);
    }
    for (int i = 0; i < 10; i++) {
        printf("Parent %d\n", i + 1);
    }
    pthread_exit(NULL);
}