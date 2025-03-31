#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define MUTEX_NUM 3

pthread_mutex_t mutex[MUTEX_NUM];

int flag = 1;

int increment(int value) {
    return (value + 1) % MUTEX_NUM;
}

int decrement(int value) {
    return (value + MUTEX_NUM - 1) % MUTEX_NUM;
}

void destroy(int end_indx) {
    for (int i = 0; i < end_indx; i++) {
        pthread_mutex_destroy(&mutex[i]);
    }
}

void* thread_body(void* arg) {
    int last = 2;
    pthread_mutex_lock(&mutex[last]);
    flag = 0;

    for (int i = 0; i < 10; i++) {
        pthread_mutex_lock(&mutex[increment(last)]);
        printf("Child.\n");

        pthread_mutex_unlock(&mutex[last]);
        last = increment(last);
    }

    return NULL;
}

int main() {
    int mc;

    for (int i = 0; i < MUTEX_NUM; i++) {
        mc = pthread_mutex_init(&mutex[i], NULL);

        if (mc != 0) {
            printf("pthread_mutex_init() returned a non-zero value.\n");
            destroy(i);

            exit(-1);
        }
    }

    int last = 0;

    pthread_mutex_lock(&mutex[last++]);
    pthread_mutex_lock(&mutex[last]);

    pthread_t thread;
    int pc = pthread_create(&thread, NULL, thread_body, NULL);

    if (pc != 0) {
        printf("pthread_create() returned a non-zero value.\n");
        destroy(MUTEX_NUM);

        exit(-1);
    }

    while (flag) {
        usleep(100000);
    }

    for (int i = 0; i < 10; i++) {
        printf("Parent.\n");

        pthread_mutex_unlock(&mutex[decrement(last)]);
        last = increment(last);
        pthread_mutex_lock(&mutex[last]);
    }

    destroy(MUTEX_NUM);

    exit(0);
}
