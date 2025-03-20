#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void *child_thread() {
    while (1) {
        write(0, "child\n", 6);
    }
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
    printf("\n2 sec left, child thread cancelled\n");
    pthreaf_exit(NULL);
}