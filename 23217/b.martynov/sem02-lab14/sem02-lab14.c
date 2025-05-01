#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <synch.h>
#include <unistd.h>
#include <string.h>

sema_t sema1;
sema_t sema2;

void* childFunc(void* param) {
    char buf[256];
    int me;

    for (int i = 10; i > 0; i--) {
        if ((me = sema_wait(&sema2)) != 0) {
            strerror_r(me, buf, sizeof(buf));
            fprintf(stderr, "Sema2 wait error: %s\n", buf);
            exit(EXIT_FAILURE);
        }

        printf("\t%d.. (child)\n", i);

        if ((me = sema_post(&sema1)) != 0) {
            strerror_r(me, buf, sizeof(buf));
            fprintf(stderr, "Sema1 post error: %s\n", buf);
            exit(EXIT_FAILURE);
        }
    }

    pthread_exit(NULL);
}

int main() {
    char buf[256];
    int me;

    if ((me = sema_init(&sema1, 0, USYNC_THREAD, NULL)) != 0) {
        strerror_r(me, buf, sizeof(buf));
        fprintf(stderr, "Init sema1 error: %s\n", buf);
        exit(EXIT_FAILURE);
    }
    if ((me = sema_init(&sema2, 0, USYNC_THREAD, NULL)) != 0) {
        strerror_r(me, buf, sizeof(buf));
        fprintf(stderr, "Init sema2 error: %s\n", buf);
        exit(EXIT_FAILURE);
    }

    pthread_t thread;
    if ((me = pthread_create(&thread, NULL, childFunc, NULL)) != 0) {
        strerror_r(me, buf, sizeof(buf));
        fprintf(stderr, "Creating thread error: %s\n", buf);
        exit(EXIT_FAILURE);
    }

    if ((me = sema_post(&sema1)) != 0) {
        strerror_r(me, buf, sizeof(buf));
        fprintf(stderr, "Start (first post) sema1 error: %s\n", buf);
        exit(EXIT_FAILURE);
    }

    for (int i = 10; i > 0; i--) {
        if ((me = sema_wait(&sema1)) != 0) {
            strerror_r(me, buf, sizeof(buf));
            fprintf(stderr, "Sema1 wait error: %s\n", buf);
            exit(EXIT_FAILURE);
        }

        printf("%d..\n", i);

        if ((me = sema_post(&sema2)) != 0) {
            strerror_r(me, buf, sizeof(buf));
            fprintf(stderr, "Sema2 post error: %s\n", buf);
            exit(EXIT_FAILURE);
        }
    }

    if ((me = sema_wait(&sema1)) != 0) {
        strerror_r(me, buf, sizeof(buf));
        fprintf(stderr, "Clear sema 1 error: %s\n", buf);
        exit(EXIT_FAILURE);
    }

    if ((me = sema_destroy(&sema1)) != 0) {
        strerror_r(me, buf, sizeof(buf));
        fprintf(stderr, "Sema1 destory error: %s\n", buf);
        exit(EXIT_FAILURE);
    }
    if ((me = sema_destroy(&sema2)) != 0) {
        strerror_r(me, buf, sizeof(buf));
        fprintf(stderr, "Sema2 destory error: %s\n", buf);
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
