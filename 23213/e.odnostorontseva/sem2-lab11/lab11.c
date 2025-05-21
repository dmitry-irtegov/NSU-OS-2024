#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

pthread_mutex_t mutex;
pthread_mutex_t parent;
pthread_mutex_t child;

volatile int init = 0;

void* func(void* arg) {
    int status;

    status = pthread_mutex_lock(&parent);
    assert(status == 0);
    init = 1;

    for (int i = 0; i < 9; i++) {
        status = pthread_mutex_lock(&child);
        assert(status == 0);
        status = pthread_mutex_unlock(&parent);
        assert(status == 0);
        status = pthread_mutex_lock(&mutex);
        assert(status == 0);
        printf("Child thread: string of text\n");
        status = pthread_mutex_unlock(&child);
        assert(status == 0);
        status = pthread_mutex_lock(&parent);
        assert(status == 0);
        status = pthread_mutex_unlock(&mutex);
        assert(status == 0);
    }
    status = pthread_mutex_lock(&child);
    assert(status == 0);
    status = pthread_mutex_unlock(&parent);
    assert(status == 0);
    status = pthread_mutex_lock(&mutex);
    assert(status == 0);
    printf("Child thread: string of text\n");
    status = pthread_mutex_unlock(&child);
    assert(status == 0);
    status = pthread_mutex_unlock(&mutex);
    assert(status == 0);

    return NULL;
}

int main(int argc, char* argv[]) {

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);

    pthread_mutex_init(&mutex, &attr);
    pthread_mutex_init(&parent, &attr);
    pthread_mutex_init(&child, &attr);
    pthread_mutexattr_destroy(&attr);

    int status;

    status = pthread_mutex_lock(&mutex);
    assert(status == 0);

    status = pthread_mutex_lock(&child);
    assert(status == 0);

    pthread_t thread;
    int res = pthread_create(&thread, NULL, func, NULL);
    if (res != 0) {
        fprintf(stderr, "%s: creating thread: %s\n", argv[0], strerror(res));
        exit(1);
    }

    while (init == 0) {
        sched_yield();
    }

    for (int i = 0; i < 9; i++) {
        printf("Parent thread: string of text\n");
        status = pthread_mutex_unlock(&child);
        assert(status == 0);
        status = pthread_mutex_lock(&parent);
        assert(status == 0);
        status = pthread_mutex_unlock(&mutex);
        assert(status == 0);
        status = pthread_mutex_lock(&child);
        assert(status == 0);
        status = pthread_mutex_unlock(&parent);
        assert(status == 0);
        status = pthread_mutex_lock(&mutex);
        assert(status == 0);
    }
    printf("Parent thread: string of text\n");
    status = pthread_mutex_unlock(&child);
    assert(status == 0);
    status = pthread_mutex_unlock(&mutex);
    assert(status == 0);

    pthread_join(thread, NULL);
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&parent);
    pthread_mutex_destroy(&child);

    return 0;
}

