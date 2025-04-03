#include <stdio.h>
#include <pthread.h>

#define ITERATIONS 5
#define M_SHARED  0
#define M_PARENT  1
#define M_CHILD   2

pthread_mutex_t mutex[3];

void *child_thread(void *arg) {
    pthread_mutex_lock(&mutex[M_CHILD]);
    for (int i = 0; i < ITERATIONS; i++) {
        pthread_mutex_lock(&mutex[M_PARENT]);
        printf("Child: %d\n", i);
        pthread_mutex_unlock(&mutex[M_CHILD]);
        pthread_mutex_lock(&mutex[M_SHARED]);
        pthread_mutex_unlock(&mutex[M_PARENT]);
        pthread_mutex_lock(&mutex[M_CHILD]);
        pthread_mutex_unlock(&mutex[M_SHARED]);
    }
    pthread_mutex_unlock(&mutex[M_CHILD]);
    return NULL;
}

int main() {
    pthread_t thread;
    pthread_mutexattr_t attr;

    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);

    for (int i = 0; i < 3; i++) {
        pthread_mutex_init(&mutex[i], &attr);
    }

    pthread_mutex_lock(&mutex[M_PARENT]);

    pthread_create(&thread, NULL, child_thread, NULL);

    for (int i = 0; i < ITERATIONS; i++) {
        printf("Parent: %d\n", i);
        pthread_mutex_lock(&mutex[M_SHARED]);
        pthread_mutex_unlock(&mutex[M_PARENT]);
        pthread_mutex_lock(&mutex[M_CHILD]);
        pthread_mutex_unlock(&mutex[M_SHARED]);
        pthread_mutex_lock(&mutex[M_PARENT]);
        pthread_mutex_unlock(&mutex[M_CHILD]);
    }

    pthread_mutex_unlock(&mutex[M_PARENT]);

    pthread_join(thread, NULL);

    for (int i = 0; i < 3; i++) {
        pthread_mutex_destroy(&mutex[i]);
    }

    pthread_mutexattr_destroy(&attr);

    return 0;
}
