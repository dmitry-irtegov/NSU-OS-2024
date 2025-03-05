#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void* childFunc(void* param) {
    for (int i = 9; i > 0; i--) {
        printf("\tUntil smth %d.. (child)\n", i);
    }
    printf("\tChild puuusk!\n");
    pthread_exit((void*)5);
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

    for (int i = 9; i > 0; i--) {
        printf("Until smth %d.. (parent)\n", i);
    }
    printf("Parent wiiie!\n");

    pthread_exit((void*)4);
}
