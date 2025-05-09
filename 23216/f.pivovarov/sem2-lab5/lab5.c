#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

void *printALotOfStrings();
void errorHandler(int code, char *errorText);
void cleanup_handler();

int main() {
    int code;
    pthread_t thread;
    pthread_attr_t attr;

    if ((code = pthread_attr_init(&attr)) != 0) {
        errorHandler(code, "Attr initialization");
    }

    if ((code = pthread_create(&thread, &attr, printALotOfStrings, NULL)) != 0) {
        errorHandler(code, "Creating thread");
    }

    sleep(2);

    if ((code = pthread_cancel(thread)) != 0) {
        errorHandler(code, "Troubles with pthread cancellation");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}

void *printALotOfStrings() {
    pthread_cleanup_push(&cleanup_handler, NULL);
    int i = 1;
    while (1) {
        printf("Print's number %d\n", i++);
        i %= __INT32_MAX__;
    }
    pthread_cleanup_pop(0);

    pthread_exit(NULL);
}

void errorHandler(int code, char *errorText) {
    char buf[256];
    strerror_r(code, buf, sizeof(buf));
    fprintf(stderr, "%s : %s\n", errorText, buf);
    exit(EXIT_FAILURE);
}

void cleanup_handler() {
    printf("Thread canceled after 2 seconds\n");
}
