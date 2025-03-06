#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define ITERATIONS 10

pthread_mutex_t mutex;
pthread_cond_t cond;
int turn = 0;

void* child_thread() {
    assert(pthread_mutex_lock(&mutex) == 0);
    for (int i = 0; i < ITERATIONS; i++) {
        while (turn != 1) {
            pthread_cond_wait(&cond, &mutex);
        }
        printf("Child: %d\n", i);
        turn = 0;
        pthread_cond_signal(&cond);
    }
    assert(pthread_mutex_unlock(&mutex) == 0);
    return NULL;
}

int main() {
    pthread_t thread;
    int errCode;
    pthread_mutexattr_t attr;
    if ((errCode = pthread_mutexattr_init(&attr)) != 0) {
        fprintf(stderr, "ERROR: Mutex attribute initialization failed: %s\n", strerror(errCode));
        exit(1);
    }
    if ((errCode = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK)) != 0) {
        fprintf(stderr, "ERROR: Mutex type set failed: %s\n", strerror(errCode));
        exit(1);
    }
    if ((errCode = pthread_mutex_init(&mutex, &attr)) != 0) {
        fprintf(stderr, "ERROR: Mutex initialization failed: %s\n", strerror(errCode));
        exit(-1);
    }
    if ((errCode = pthread_cond_init(&cond, NULL)) != 0) {
        fprintf(stderr, "ERROR: Condition initialization failed: %s\n", strerror(errCode));
        exit(-1);
    }
    if ((errCode = pthread_create(&thread, NULL, child_thread, NULL)) != 0) {
        fprintf(stderr, "ERROR: Thread creation failed: %s\n", strerror(errCode));
        exit(1);
    }
    assert(pthread_mutex_lock(&mutex) == 0);
    for (int i = 0; i < ITERATIONS; i++) {
        while (turn != 0) {
            pthread_cond_wait(&cond, &mutex);
        }
        printf("Parent: %d\n", i);
        turn = 1;
        pthread_cond_signal(&cond);
    }
    assert(pthread_mutex_unlock(&mutex) == 0);
    pthread_join(thread, NULL);
    pthread_mutex_destroy(&mutex);
    pthread_mutexattr_destroy(&attr);
    pthread_cond_destroy(&cond);
    exit(0);
}
