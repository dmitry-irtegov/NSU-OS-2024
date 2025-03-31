#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void* childFunc(void* paramPamPam) {
    for (int i = 9; i > 0; i--) {
        printf("Until smth %d.. (child)\n", i);
    }
    printf("Child puuusk!\n");
    pthread_exit(NULL);
}

int main() {
    pthread_t thread;
    int thrErr = pthread_create(&thread, NULL, childFunc, NULL);
    if (thrErr != 0) {
        char buf[256];
        strerror_r(thrErr, buf, sizeof(buf));
        fprintf(stderr, "Creating thread error: %s\n", buf);
        exit(EXIT_FAILURE);
    }

    void* ret;
    thrErr = pthread_join(thread, &ret);
    if (thrErr != 0) {
        char buf[256];
        strerror_r(thrErr, buf, sizeof(buf));
        fprintf(stderr, "Thread join error: %s\n", buf);
        exit(EXIT_FAILURE);
    }

    for (int i = 9; i > 0; i--) {
        printf("Until smth %d.. (parent)\n", i);
    }
    printf("Parent wiiie!\n");

    pthread_exit(NULL);
}

