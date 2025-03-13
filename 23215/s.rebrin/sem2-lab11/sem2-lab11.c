#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <stdatomic.h>

#define M 3

pthread_mutex_t mutex[M];
atomic_int turn = 0; // 0 - main 

int add(int a, int b) {
    return (a + b) % M;
}

void* thread_body(void* param) {
    int cur = 2;

    int lock_result1 = pthread_mutex_lock(&mutex[cur]);
    if (lock_result1 != 0) {
        fprintf(stderr, "pthread_mutex_lock error in thread: %s\n", strerror(lock_result1));
        pthread_exit(NULL);
    }

    int unlock_result;
    cur = add(cur, 1);
    atomic_store(&turn, 1);
    for (int i = 0; i < 10; i++) {
        lock_result1 = pthread_mutex_lock(&mutex[cur]);
        if (lock_result1 != 0) {
            fprintf(stderr, "pthread_mutex_lock error in thread: %s\n", strerror(lock_result1));
            pthread_exit(NULL);
        }

        printf("left\n");
        atomic_store(&turn, 0);
        int unlock_result = pthread_mutex_unlock(&mutex[add(cur, M - 1)]);
        if (unlock_result != 0) {
            fprintf(stderr, "pthread_mutex_unlock error in thread: %s\n", strerror(unlock_result));
            pthread_exit(NULL);
        }
        cur = add(cur, 1);
    }

    unlock_result = pthread_mutex_unlock(&mutex[add(cur, M - 1)]);
    if (unlock_result != 0) {
        fprintf(stderr, "pthread_mutex_unlock error in thread: %s\n", strerror(unlock_result));
        pthread_exit(NULL);
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    pthread_t thread;
    int code;
    int mc;

    // Инициализация атрибутов мьютекса
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);

    for (int i = 0; i < M; i++) {
        // Инициализация мьютекса с атрибутом PTHREAD_MUTEX_ERRORCHECK
        mc = pthread_mutex_init(&mutex[i], &attr);

        if (mc != 0) {
            printf("pthread_mutex_init error.\n");
            for (int ii = 0; ii < i; ii++) {
                pthread_mutex_destroy(&mutex[ii]);
            }
            pthread_mutexattr_destroy(&attr);
            exit(-1);
        }
    }


    pthread_mutexattr_destroy(&attr);

    int cur = 0;
    int unlock_result1;
    int lock_result = pthread_mutex_lock(&mutex[cur++]);
    if (lock_result != 0) {
        fprintf(stderr, "pthread_mutex_lock error in thread: %s\n", strerror(lock_result));
        pthread_exit(NULL);
    }

    if ((code = pthread_create(&thread, NULL, thread_body, NULL)) != 0) {
        char buf[256];
        strerror_r(code, buf, sizeof(buf));
        fprintf(stderr, "%s: creating thread: %s\n", argv[0], buf);
        for (int i = 0; i < M; i++) {
            pthread_mutex_destroy(&mutex[i]);
        }
        exit(1);
    }
    else {
        while (!atomic_load(&turn));
        for (int i = 0; i < 10; i++) {
            lock_result = pthread_mutex_lock(&mutex[cur]);
            if (lock_result != 0) {
                fprintf(stderr, "pthread_mutex_lock error in thread: %s\n", strerror(lock_result));
                pthread_exit(NULL);
            }
            

            printf("right\n");


            unlock_result1 = pthread_mutex_unlock(&mutex[add(cur, M - 1)]);
            if (unlock_result1 != 0) {
                fprintf(stderr, "pthread_mutex_unlock error in thread: %s\n", strerror(unlock_result1));
                pthread_exit(NULL);
            }
            cur = add(cur, 1);
        }
    }

    unlock_result1 = pthread_mutex_unlock(&mutex[add(cur, M - 1)]);
    if (unlock_result1 != 0) {
        fprintf(stderr, "pthread_mutex_unlock error in thread: %s\n", strerror(unlock_result1));
        pthread_exit(NULL);
    }
       pthread_join(thread, NULL);
    for (int ii = 0; ii < M; ii++) {
        pthread_mutex_destroy(&mutex[ii]);
    }
    pthread_exit(NULL);
}