#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

pthread_mutex_t m;
pthread_cond_t cond;

int flag = 0;

void* childFunc(void* param) {
    char buf[256];
    int me;

    for (int i = 10; i > 0; i--) {
        if ((me = pthread_mutex_lock(&m)) != 0) {
            strerror_r(me, buf, sizeof(buf));
            fprintf(stderr, "Lock m error: %s\n", buf);
            exit(EXIT_FAILURE);
        }

        while (flag != 0) {
            pthread_cond_wait(&cond, &m);
        }

        printf("\t%d.. (child)\n", i);
        flag = 1;

        if ((me = pthread_cond_signal(&cond)) != 0) {
            strerror_r(me, buf, sizeof(buf));
            fprintf(stderr, "Child cond signal error: %s\n", buf);
            exit(EXIT_FAILURE);
        }

        if ((me = pthread_mutex_unlock(&m)) != 0) {
            strerror_r(me, buf, sizeof(buf));
            fprintf(stderr, "Unlock m error: %s\n", buf);
            exit(EXIT_FAILURE);
        }
    }

    pthread_exit(NULL);
}

int main() {
    char buf[256];
    int me;

    pthread_mutexattr_t attr;
    if ((me = pthread_mutexattr_init(&attr)) != 0) {
        strerror_r(me, buf, sizeof(buf));
        fprintf(stderr, "Mutex attr init error: %s\n", buf);
        exit(EXIT_FAILURE);
    }
    if ((me = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK)) != 0) {
        strerror_r(me, buf, sizeof(buf));
        fprintf(stderr, "Mutex type set error: %s\n", buf);
        exit(EXIT_FAILURE);
    }

    if ((me = pthread_mutex_init(&m, &attr) != 0)) {
        strerror_r(me, buf, sizeof(buf));
        fprintf(stderr, "Mutex m init error: %s\n", buf);
        exit(EXIT_FAILURE);
    }

    if ((me = pthread_cond_init(&cond, NULL)) != 0) {
        strerror_r(me, buf, sizeof(buf));
        fprintf(stderr, "Cond init error: %s\n", buf);
        exit(EXIT_FAILURE);
    }

    flag = 1;

    pthread_t thread;
    if ((me = pthread_create(&thread, NULL, childFunc, NULL)) != 0) {
        strerror_r(me, buf, sizeof(buf));
        fprintf(stderr, "Creating thread error: %s\n", buf);
        exit(EXIT_FAILURE);
    }

    for (int i = 10; i > 0; i--) {
        if ((me = pthread_mutex_lock(&m)) != 0) {
            strerror_r(me, buf, sizeof(buf));
            fprintf(stderr, "Lock m error: %s\n", buf);
            exit(EXIT_FAILURE);
        }

        while (flag == 0) {
            pthread_cond_wait(&cond, &m);
        }

        printf("%d..\n", i);
        flag = 0;

        if ((me = pthread_cond_signal(&cond)) != 0) {
            strerror_r(me, buf, sizeof(buf));
            fprintf(stderr, "Parent cond signal error: %s\n", buf);
            exit(EXIT_FAILURE);
        }

        if ((me = pthread_mutex_unlock(&m)) != 0) {
            strerror_r(me, buf, sizeof(buf));
            fprintf(stderr, "Unlock m error: %s\n", buf);
            exit(EXIT_FAILURE);
        }
    }

    void* ret;
    if ((me = pthread_join(thread, &ret)) != 0) {
        strerror_r(me, buf, sizeof(buf));
        fprintf(stderr, "Thread join error: %s\n", buf);
        exit(EXIT_FAILURE);
    }

    if ((me = pthread_mutex_destroy(&m)) != 0) {
        strerror_r(me, buf, sizeof(buf));
        fprintf(stderr, "Mutex m destory error: %s\n", buf);
        exit(EXIT_FAILURE);
    }

    if ((me = pthread_mutexattr_destroy(&attr)) != 0) {
        strerror_r(me, buf, sizeof(buf));
        fprintf(stderr, "Mutex attr destory error: %s\n", buf);
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
