#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

void* thread_func(void* arg) {
    while (1) {
        puts((char*)arg);
        usleep(10000);
    }

    pthread_exit(NULL);
}

int main() {
    pthread_t thread;
    char* text = "this is the text";

    int code = 0;
    if ((code = pthread_create(&thread, NULL, thread_func, (void*)text)) != 0) {
        fprintf(stderr, "pthread_create error: %d \n", code);
        exit(EXIT_FAILURE);
    }
    
    sleep(2);

    if ((code = pthread_cancel(thread)) != 0) {
        fprintf(stderr, "pthread_cancel error: %d \n", code);
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}