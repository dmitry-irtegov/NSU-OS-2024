#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

void my_assert(int val) {
    if (val != 0) {
        fprintf(stderr, "Assertion failed: value is not zero (value = %d)\n", val);
        abort();
    }
}

pthread_mutex_t mutex[3];
volatile int flag = 0;

void* thread_body() {
    my_assert(pthread_mutex_lock(&mutex[2]));

    flag = 1;

    for (int i = 0; i < 10; i++) {
        my_assert(pthread_mutex_lock(&mutex[1]));
        my_assert(pthread_mutex_unlock(&mutex[2]));
        my_assert(pthread_mutex_lock(&mutex[0]));

        printf("child\n");

        my_assert(pthread_mutex_unlock(&mutex[1]));
        my_assert(pthread_mutex_lock(&mutex[2]));
        my_assert(pthread_mutex_unlock(&mutex[0]));
    }

    my_assert(pthread_mutex_unlock(&mutex[2]));

    return NULL;
}

int main() {
    pthread_t thread;
    int code;   

    pthread_mutexattr_t mutex_attr;
    pthread_mutexattr_init(&mutex_attr);
    code = pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_ERRORCHECK);
    if (code != 0) {
        fprintf(stderr, "mutex_attr settype fail: %s\n", strerror(code));
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < 3; i++) {
        pthread_mutex_init(&mutex[i], &mutex_attr);
    }
    pthread_mutexattr_destroy(&mutex_attr);

    my_assert(pthread_mutex_lock(&mutex[0]));
    my_assert(pthread_mutex_lock(&mutex[1]));
    
    pthread_create(&thread, NULL, thread_body, NULL);

    while (!flag) { sched_yield(); }

    for (int i = 0; i < 10; i++) {
        printf("parent\n");
        my_assert(pthread_mutex_unlock(&mutex[1]));
        my_assert(pthread_mutex_lock(&mutex[2]));
        my_assert(pthread_mutex_unlock(&mutex[0]));

        my_assert(pthread_mutex_lock(&mutex[1]));
        my_assert(pthread_mutex_unlock(&mutex[2]));
        my_assert(pthread_mutex_lock(&mutex[0]));
    }

    my_assert(pthread_mutex_unlock(&mutex[1]));
    my_assert(pthread_mutex_unlock(&mutex[0]));

    pthread_join(thread, NULL);
    
    for (int i = 0; i < 3; i++) {
        my_assert(pthread_mutex_destroy(&mutex[i]));
    }

    pthread_exit(NULL);
}
