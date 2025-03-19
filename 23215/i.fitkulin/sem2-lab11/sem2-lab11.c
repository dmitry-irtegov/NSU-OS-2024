#include <stdio.h>  
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>


pthread_mutex_t mutex[3];

volatile int flag = 0;

void* thread_body() {

    assert(pthread_mutex_lock(&mutex[2]) == 0);

    flag = 1;

    for (int i = 0; i < 10; i++) {
        assert(pthread_mutex_lock(&mutex[1]) == 0);
        assert(pthread_mutex_unlock(&mutex[2]) == 0);
        assert(pthread_mutex_lock(&mutex[0]) == 0);

        printf("child\n");

        assert(pthread_mutex_unlock(&mutex[1]) == 0);
        assert(pthread_mutex_lock(&mutex[2]) == 0);
        assert(pthread_mutex_unlock(&mutex[0]) == 0);
    }

    assert(pthread_mutex_unlock(&mutex[2]) == 0);

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

    assert(pthread_mutex_lock(&mutex[0]) == 0);
    assert(pthread_mutex_lock(&mutex[1]) == 0);

    pthread_create(&thread, NULL, thread_body, NULL);

    while (!flag) { sched_yield(); }

    for (int i = 0; i < 10; i++) {
        printf("parent\n");
        assert(pthread_mutex_unlock(&mutex[1]) == 0);
        assert(pthread_mutex_lock(&mutex[2]) == 0);
        assert(pthread_mutex_unlock(&mutex[0]) == 0);

        assert(pthread_mutex_lock(&mutex[1]) == 0);
        assert(pthread_mutex_unlock(&mutex[2]) == 0);
        assert(pthread_mutex_lock(&mutex[0]) == 0);
    }

    assert(pthread_mutex_unlock(&mutex[1]) == 0);
    assert(pthread_mutex_unlock(&mutex[0]) == 0);

    pthread_join(thread, NULL);
    
    for (int i = 0; i < 3; i++) {
        assert(pthread_mutex_destroy(&mutex[i]) == 0);
    }

    pthread_exit(NULL);
} 
