#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <stdatomic.h>

#define M 3

pthread_mutex_t mutexes[M];
atomic_int who_turn = 0; // 0 — main thread turn

int next_index(int idx, int step) {
    return (idx + step) % M;
}

void* child_thread(void* arg) {
    int pos = 2;

    if (pthread_mutex_lock(&mutexes[pos]) != 0) {
        perror("thread lock init");
        pthread_exit(NULL);
    }

    pos = next_index(pos, 1);
    atomic_store(&who_turn, 1);

    for (int i = 0; i < 10; ++i) {
        if (pthread_mutex_lock(&mutexes[pos]) != 0) {
            perror("thread lock loop");
            pthread_exit(NULL);
        }

        printf("    child: left\n");

        atomic_store(&who_turn, 0);

        if (pthread_mutex_unlock(&mutexes[next_index(pos, M - 1)]) != 0) {
            perror("thread unlock loop");
            pthread_exit(NULL);
        }

        pos = next_index(pos, 1);
    }

    if (pthread_mutex_unlock(&mutexes[next_index(pos, M - 1)]) != 0) {
        perror("thread unlock final");
        pthread_exit(NULL);
    }

    return NULL;
}

int main(int argc, char* argv[]) {
    pthread_t thread_id;
    pthread_mutexattr_t attr;
    int rc;

    if (pthread_mutexattr_init(&attr) != 0 ||
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK) != 0) {
        fprintf(stderr, "mutex attr init/settype failed\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < M; ++i) {
        if ((rc = pthread_mutex_init(&mutexes[i], &attr)) != 0) {
            fprintf(stderr, "mutex init error %d\n", rc);
            for (int j = 0; j < i; ++j) {
                pthread_mutex_destroy(&mutexes[j]);
            }
            pthread_mutexattr_destroy(&attr);
            exit(EXIT_FAILURE);
        }
    }
    pthread_mutexattr_destroy(&attr);

    int pos = 0;
    if (pthread_mutex_lock(&mutexes[pos++]) != 0) {
        perror("main initial lock");
        pthread_exit(NULL);
    }

    rc = pthread_create(&thread_id, NULL, child_thread, NULL);
    if (rc != 0) {
        char errbuf[256];
        strerror_r(rc, errbuf, sizeof(errbuf));
        fprintf(stderr, "thread creation failed: %s\n", errbuf);
        for (int i = 0; i < M; ++i) {
            pthread_mutex_destroy(&mutexes[i]);
        }
        exit(EXIT_FAILURE);
    }

    while (!atomic_load(&who_turn));

    for (int i = 0; i < 10; ++i) {
        if (pthread_mutex_lock(&mutexes[pos]) != 0) {
            perror("main loop lock");
            pthread_exit(NULL);
        }

        printf("main:    right\n");

        if (pthread_mutex_unlock(&mutexes[next_index(pos, M - 1)]) != 0) {
            perror("main loop unlock");
            pthread_exit(NULL);
        }

        pos = next_index(pos, 1);
    }

    if (pthread_mutex_unlock(&mutexes[next_index(pos, M - 1)]) != 0) {
        perror("main unlock final");
        pthread_exit(NULL);
    }

    pthread_join(thread_id, NULL);

    for (int i = 0; i < M; ++i) {
        pthread_mutex_destroy(&mutexes[i]);
    }

    pthread_exit(NULL);
}