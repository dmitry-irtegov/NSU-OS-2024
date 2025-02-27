#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>

#define ITERATIONS 10

sem_t sem1, sem2;

void *child_thread() {
    for (int i = 0; i < ITERATIONS; i++) {
        sem_wait(&sem2);
        printf("Child: %d\n", i);
        sem_post(&sem1);
    }
    return NULL;
}

int main() {
    pthread_t thread;
    int errCode;
    if ((errCode = sem_init(&sem1, 0, 1)) != 0) {
        fprintf(stderr, "ERROR: Unable to initialize 1st semaphore: %s\n", strerror(errCode));
        exit(1);
    }
    if ((errCode = sem_init(&sem2, 0, 0)) != 0) {
        fprintf(stderr, "ERROR: Unable to initialize 2st semaphore: %s\n", strerror(errCode));
        exit(1);
    }
    if ((errCode = pthread_create(&thread, NULL, child_thread, NULL)) != 0) {
        fprintf(stderr, "ERROR: Thread creation failed: %s\n", strerror(errCode));
        exit(1);
    }
    for (int i = 0; i < ITERATIONS; i++) {
        sem_wait(&sem1);
        printf("Parent: %d\n", i);
        sem_post(&sem2);
    }
    pthread_join(thread, NULL);
    sem_destroy(&sem1);
    sem_destroy(&sem2);
    exit(0);
}
