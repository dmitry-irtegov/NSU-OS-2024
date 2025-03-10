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

    pthread_mutex_lock(&mutex[cur]);
    cur = add(cur, 1);
    atomic_store(&turn, 1);
    for (int i = 0; i < 10; i++) {
        pthread_mutex_lock(&mutex[cur]);

        printf("left\n");
        atomic_store(&turn, 0);
        pthread_mutex_unlock(&mutex[add(cur, M - 1)]);
        cur = add(cur, 1);
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    pthread_t thread;
    int code;
    int mc;

    for (int i = 0; i < M; i++) {
        mc = pthread_mutex_init(&mutex[i], NULL);

        if (mc != 0) {
            printf("pthread_mutex_init error.\n");
            for (int ii = 0; ii < i; ii++) {
                pthread_mutex_destroy(&mutex[ii]);
            }

            exit(-1);
        }
    }

    int cur = 0;

    pthread_mutex_lock(&mutex[cur++]);

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
            pthread_mutex_lock(&mutex[cur]);

            printf("right\n");

            pthread_mutex_unlock(&mutex[add(cur, M - 1)]);
            cur = add(cur, 1);
        }
    }
    pthread_mutex_unlock(&mutex[add(cur, M - 1)]);
    pthread_join(thread, NULL);
    for (int ii = 0; ii < M; ii++) {
        pthread_mutex_destroy(&mutex[ii]);
    }
    pthread_exit(NULL);
}