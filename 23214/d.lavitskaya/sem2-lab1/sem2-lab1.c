#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

void *print_messages(void *arg) {
    for (int i = 0; i < 10; i++) {
        printf("[Thread] Message %d\n", i + 1);
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    pthread_t thread;
    int code;

    code = pthread_create(&thread, NULL, print_messages, NULL);
    if (code != 0) {
        char buf[256];
        strerror_r(code, buf, sizeof(buf));
        fprintf(stderr, "%s: creating thread: %s\n", argv[0], buf);
        exit(1);
    }

    for (int i = 0; i < 10; i++) {
        printf("[Main] Message %d\n", i + 1);
    }
    
    pthread_exit(NULL);
}
