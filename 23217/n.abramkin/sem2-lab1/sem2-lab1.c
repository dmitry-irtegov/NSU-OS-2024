#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

void* thread_function(void* arg) {
    for (int i = 0; i < 10; i++) {
        printf("[Child Thread] Line %d\n", i + 1);
        usleep(100000);
    }
    return NULL;
}

int main() {
    pthread_t thread;
    
    pthread_create(&thread, NULL, thread_function, NULL);

    for (int i = 0; i < 10; i++) {
        printf("[Main Thread] Line %d\n", i + 1);
        usleep(100000);
    }

    pthread_exit(NULL);
}
