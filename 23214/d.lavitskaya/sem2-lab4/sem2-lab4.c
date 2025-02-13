#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

void *print_messages(void *arg) {
    while (1) {
        printf("[Thread] Running...\n");
    }
    return NULL;  
}

int main(int argc, char *argv[]) {
    pthread_t thread;
    int code;
    char* buf;

    code = pthread_create(&thread, NULL, print_messages, NULL);
    if (code != 0) {
        buf = strerror(code);
        fprintf(stderr, "%s: creating thread: %s\n", argv[0], buf);
        exit(1);
    }

    sleep(2);

    printf("[Main] Cancelling thread...\n");
    code = pthread_cancel(thread);
    if (code != 0) {
        buf = strerror(code);
        fprintf(stderr, "%s: canceling thread: %s\n", argv[0], buf);
        exit(1);
    }

    code = pthread_join(thread, NULL);
    if (code != 0) {
        buf = strerror(code);
        fprintf(stderr, "%s: joining thread: %s\n", argv[0], buf);
        exit(1);
    }

    printf("[Main] Thread cancelled, exiting program.\n");
    return 0;
}
