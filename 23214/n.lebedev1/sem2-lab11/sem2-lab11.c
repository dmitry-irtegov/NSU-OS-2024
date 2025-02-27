#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <string.h>

#define ITERATIONS 10
#define MUTEX_COUNT 3

#define MUTEX_COMMON  0
#define MUTEX_PARENT  1
#define MUTEX_CHILD   2

pthread_mutex_t mutex[MUTEX_COUNT];

void *child_thread() {
    assert(pthread_mutex_lock(&mutex[MUTEX_CHILD]) == 0);
    for (int i = 0; i < ITERATIONS; i++) {
        assert(pthread_mutex_lock(&mutex[MUTEX_PARENT]) == 0);
        printf("Child: %d\n", i);
        assert(pthread_mutex_unlock(&mutex[MUTEX_CHILD]) == 0);
        assert(pthread_mutex_lock(&mutex[MUTEX_COMMON]) == 0);
        assert(pthread_mutex_unlock(&mutex[MUTEX_PARENT]) == 0);
        assert(pthread_mutex_lock(&mutex[MUTEX_CHILD]) == 0);
        assert(pthread_mutex_unlock(&mutex[MUTEX_COMMON]) == 0);
    }
    assert(pthread_mutex_unlock(&mutex[MUTEX_CHILD]) == 0);
    return NULL;
}

int main() {
    pthread_t thread;
    int errCode;
    pthread_mutexattr_t attr;
    if ((errCode = pthread_mutexattr_init(&attr)) != 0) {
        fprintf(stderr, "ERROR: Mutex attribute initialization failed: %s\n", strerror(errCode));
        exit(-1);
    }
    if ((errCode = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK)) != 0) {
        fprintf(stderr, "ERROR: Mutex type set failed: %s\n", strerror(errCode));
        exit(-1);
    }
    for (int i = 0; i < MUTEX_COUNT; i++) {
        if ((errCode = pthread_mutex_init(&mutex[i], &attr)) != 0) {
            fprintf(stderr, "ERROR: Mutex initialization failed: %s\n", strerror(errCode));
            exit(-1);
        }
    }

    assert(pthread_mutex_lock(&mutex[MUTEX_PARENT]) == 0);

    if ((errCode = pthread_create(&thread, NULL, child_thread, NULL)) != 0) {
        fprintf(stderr, "ERROR: Thread creation failed: %s\n", strerror(errCode));
        exit(-1);
    }

    for (int i = 0; i < ITERATIONS; i++) {
        printf("Parent: %d\n", i);
        assert(pthread_mutex_lock(&mutex[MUTEX_COMMON]) == 0);
        assert(pthread_mutex_unlock(&mutex[MUTEX_PARENT]) == 0);
        assert(pthread_mutex_lock(&mutex[MUTEX_CHILD]) == 0);
        assert(pthread_mutex_unlock(&mutex[MUTEX_COMMON]) == 0);
        assert(pthread_mutex_lock(&mutex[MUTEX_PARENT]) == 0);
        assert(pthread_mutex_unlock(&mutex[MUTEX_CHILD]) == 0);
    }
    assert(pthread_mutex_unlock(&mutex[MUTEX_PARENT]) == 0);

    if ((errCode = pthread_join(thread, NULL)) != 0) {
        fprintf(stderr, "ERROR: Thread join failed: %s\n", strerror(errCode));
        exit(-1);
    }

    for (int i = 0; i < MUTEX_COUNT; i++) {
        if ((errCode = pthread_mutex_destroy(&mutex[i])) != 0) {
            fprintf(stderr, "ERROR: Mutex destruction failed: %s\n", strerror(errCode));
            exit(-1);
        }
    }
    pthread_mutexattr_destroy(&attr);
    exit(0);
}