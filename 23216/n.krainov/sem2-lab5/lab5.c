#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

void handler(void* arg) {
    puts(arg);
}

void* thread_func(void* arg) {
    pthread_cleanup_push(handler, "i'm dead");
    while (1) {
        puts((char*)arg);
    }

    pthread_cleanup_pop(0);
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

    pthread_exit(NULL);
}