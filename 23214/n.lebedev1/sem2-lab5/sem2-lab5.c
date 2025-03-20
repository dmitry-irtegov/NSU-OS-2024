#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void child_handler() {
    printf("\nChild canceled!\n");
}

void *child_thread() {
    pthread_cleanup_push(child_handler, NULL);
    while (1) {
        write(0, "child\n", 6);
    }
    pthread_cleanup_pop(0);
    return NULL;
}

int main() {
    pthread_t thread;
    int errCode;
    if ((errCode = pthread_create(&thread, NULL, child_thread, NULL)) != 0) {
        fprintf(stderr, "ERROR: Thread creation failed: %s\n", strerror(errCode));
        exit(-1);
    }
    sleep(2);
    if ((errCode = pthread_cancel(thread)) != 0) {
        fprintf(stderr, "ERROR: Thread canceling failed: %s\n", strerror(errCode));
        exit(-1);
    }
    pthread_exit(NULL);
}
