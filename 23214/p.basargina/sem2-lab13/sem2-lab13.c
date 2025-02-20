#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

pthread_mutex_t mutex;
pthread_cond_t cond;
int turn = 0;

void* child_thread(void* arg) {
    assert(pthread_mutex_lock(&mutex) == 0);
    for (int i = 0; i < 10; i++) {
        while (turn != 1) {
            pthread_cond_wait(&cond, &mutex);
        }
        printf("Child thread: %d  :-)\n", i + 1);

        turn = 0;

        pthread_cond_signal(&cond);
    }
    assert(pthread_mutex_unlock(&mutex) == 0);

    pthread_exit(NULL);
}

int main() {
    pthread_t thread;
    int errCode;
    pthread_mutexattr_t attr;

    if ((errCode = pthread_mutexattr_init(&attr)) != 0) {
        fprintf(stderr, "Init mutex attributes failed: %s\n", strerror(errCode));
        exit(1);
    }
    if ((errCode = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK)) != 0) {
        fprintf(stderr, "Set mutex type failed: %s\n", strerror(errCode));
        exit(1);
    }
    pthread_mutex_init(&mutex, &attr);
    pthread_cond_init(&cond, NULL);

    if ((errCode = pthread_create(&thread, NULL, child_thread, NULL)) != 0) {
        fprintf(stderr, "Create thread failed: %s\n", strerror(errCode));
        exit(1);
    }

    assert(pthread_mutex_lock(&mutex) == 0);
    for (int i = 0; i < 10; i++) {
        while (turn != 0) {
            pthread_cond_wait(&cond, &mutex);
        }
        printf("Parent thread: %d  :-)\n", i + 1);

        turn = 1;

        pthread_cond_signal(&cond);
    }
    assert(pthread_mutex_unlock(&mutex) == 0);

    pthread_join(thread, NULL);

    exit(0);
}
