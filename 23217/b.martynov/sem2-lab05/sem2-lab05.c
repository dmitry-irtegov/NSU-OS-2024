#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void printer() {
    printf("Aaaaaaaa, why are you killing me?! :((((\n");
}

void* childFunc(void* paramPamPam) {
    pthread_cleanup_push(&printer, NULL);

    for (int i = 9; i > 0; i--) {
        printf("Until smth %d.. (child)\n", i);
        usleep(500 * 1000);
    }
    printf("Child puuusk!\n");
    pthread_cleanup_pop(0);
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

    sleep(2);

    void* ret;
    thrErr = pthread_cancel(thread);
    if (thrErr != 0) {
        char buf[256];
        strerror_r(thrErr, buf, sizeof(buf));
        fprintf(stderr, "Thread cancel error: %s\n", buf);
        exit(EXIT_FAILURE);
    }

    pthread_exit(NULL);
}
