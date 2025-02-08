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

void my_perror(char* text, int code) {
    fprintf(stderr, "%s: %s \n", text, strerror(code));
}

int main() {
    pthread_t thread;
    char* text = "this is the text";

    int code = 0;
    if ((code = pthread_create(&thread, NULL, thread_func, (void*)text)) != 0) {
        my_perror("pthread_create failed", code);
        exit(EXIT_FAILURE);
    }
    
    sleep(2);

    if ((code = pthread_cancel(thread)) != 0) {
        my_perror("pthread_cancel failed", code);
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}