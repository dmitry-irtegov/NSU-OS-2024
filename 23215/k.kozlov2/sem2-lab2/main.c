#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

void * thread_body(void *param) {
    for(int i = 0; i < 10; i++) 
        printf("Child thread\n");
    return NULL;
}

int main(int argc, char *argv[]) {
    pthread_t thread;
    int code1, code2;

    code1 = pthread_create(&thread, NULL, thread_body, NULL);
    if (code1 != 0) {
        char buf[256];
        strerror_r(code1, buf, sizeof buf);
        fprintf(stderr, "%s: pthread_create: %s\n", argv[0], buf);
        exit(EXIT_FAILURE);
    }

    code2 = pthread_join(thread, NULL);
    if (code2 != 0) {
        char buf[256];
        strerror_r(code2, buf, sizeof buf);
        fprintf(stderr, "%s: pthread_join: %s\n", argv[0], buf);
        exit(EXIT_FAILURE);
    }

    for(int i = 0; i < 10; i++) 
        printf("Parent thread\n");

    pthread_exit(NULL);
}
