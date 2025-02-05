#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void* thread_func(void* args) {
    for (int i = 0; i < 10; i++) {
        printf("Hello from child thread!\n");
    }
    return NULL;
}

int main() {
    pthread_t th;
    pthread_create(&th, NULL, thread_func, NULL);

    pthread_join(th, NULL);

    for (int i = 0; i < 10; i++) {
        printf("Hello from main thread!\n");
    }

    pthread_exit(NULL); 
}
