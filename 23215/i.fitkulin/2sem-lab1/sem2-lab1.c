#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

void* thread_body(void* param) {
    for (int i = 0; i < 10; i++) {
        printf("child\n");
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    pthread_t thread;
    
    int code = pthread_create(&thread, NULL, thread_body, NULL);
    if (code != 0) {
        char buf[256];
        strerror_r(code, buf, sizeof(buf));
        fprintf(stderr, "%s: creating thread: %s\n", argv[0], buf);
        exit(EXIT_FAILURE);
    }
    
    for (int i = 0; i < 10; i++) {
        printf("parent\n");
    }

    pthread_exit(NULL);
} 