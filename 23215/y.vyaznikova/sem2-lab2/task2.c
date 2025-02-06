#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

void* thread_body(void* param) {
    for (int i = 1; i < 11; i++) {
        printf("Child thread %d\n", i);
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    pthread_t thread;
    int code;
    
    code = pthread_create(&thread, NULL, thread_body, NULL);
    if (code != 0) {
        char buf[256];
        strerror_r(code, buf, sizeof(buf));
        fprintf(stderr, "%s: creating thread: %s\n", argv[0], buf);
        exit(1);
    }

    pthread_join(thread, NULL);
    
    for (int i = 1; i < 11; i++) {
        printf("Parent thread %d\n", i);
    }
    
    pthread_exit(NULL);   
}