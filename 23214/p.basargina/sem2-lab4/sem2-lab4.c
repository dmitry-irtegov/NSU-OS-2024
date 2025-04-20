#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

void* thread_function(void* arg) {
    while(1) {
        write(1, "Child thread\n", 13);
        usleep(100000);
    }
}

int main() {
    pthread_t thread;
    int errCode;

    if ((errCode = pthread_create(&thread, NULL, thread_function, NULL)) != 0) {
        char* buf = strerror(errCode);
        fprintf(stderr, "Failed to create thread: %s\n", buf);
        exit(1);
    }

    sleep(2);

    printf("Cancel attempt.\n");

    if ((errCode = pthread_cancel(thread)) != 0) {
        char* buf = strerror(errCode);
        fprintf(stderr, "Failed to cancel thread: %s\n", buf);
        exit(1);
    }

    if ((errCode = pthread_join(thread, NULL)) != 0) {
        char* buf = strerror(errCode);
        fprintf(stderr, "Failed to join thread: %s\n", buf);
        exit(1);
    }

    printf("Thread has been cancelled.\n");

    exit(0);
}
