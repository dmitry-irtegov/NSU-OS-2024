#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

void *thread_body() {
    for (int i = 0; i < 10; i++) {
        printf("Child %d\n", i + 1);
    }
    return 0;
}

int main() {
    pthread_t thread;
    int err_code;
    if ((err_code = pthread_create(&thread, NULL, thread_body, NULL)) != 0) {
        char buf[256];
        strerror_r(err_code, buf, sizeof(buf));
        fprintf(stderr, "Error with creating thread: %s\n", buf);
        exit(-1);
    }
    for (int i = 0; i < 10; i++) {
        printf("Parent %d\n", i + 1);
    }
    pthread_exit(0);
}