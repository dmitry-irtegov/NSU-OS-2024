#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define SEMAPHORE_NUM 2

sem_t semaphore[SEMAPHORE_NUM];

void destroy(int end_indx) {
    for (int i = 0; i < end_indx; i++) {
        sem_destroy(&semaphore[i]);
    }
}

void* thread_body(void* arg) {
    for (int i = 0; i < 10; i++) {
        sem_wait(&semaphore[1]);
        printf("Child.\n");
        sem_post(&semaphore[0]);
    }

    return NULL;
}

int main() {
    int sc;

    for (int i = 0; i < SEMAPHORE_NUM; i++) {
        sc = sem_init(&semaphore[i], 0, 0);

        if (sc == -1) {
            printf("sem_init() returned -1.\n");
            destroy(i);

            exit(-1);
        }
    }

    pthread_t thread;
    int pc = pthread_create(&thread, NULL, thread_body, NULL);

    if (pc != 0) {
        printf("pthread_create() returned a non-zero value.\n");
        destroy(SEMAPHORE_NUM);

        exit(-1);
    }

    for (int i = 0; i < 10; i++) {
        printf("Parent.\n");
        sem_post(&semaphore[1]);
        sem_wait(&semaphore[0]);
    }

    destroy(SEMAPHORE_NUM);

    exit(0);
}
