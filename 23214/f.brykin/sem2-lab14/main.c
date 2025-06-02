#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define LINES_COUNT 10

sem_t parent_sem;
sem_t child_sem;

void* thread_function(void* arg) {
    for (int i = 0; i < LINES_COUNT; i++) {
        sem_wait(&child_sem);
        printf("Child thread: line %d\n", i);
        sem_post(&parent_sem);
    }
    return NULL;
}

int main() {
    pthread_t thread;
    sem_init(&parent_sem, 0, 1);
    sem_init(&child_sem, 0, 0);
    int result = pthread_create(&thread, NULL, thread_function, NULL);
    if (result != 0) {
        perror("Error creating thread");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < LINES_COUNT; i++) {
        sem_wait(&parent_sem);
        printf("Parent thread: line %d\n", i);
        sem_post(&child_sem);
    }
    pthread_join(thread, NULL);
    sem_destroy(&parent_sem);
    sem_destroy(&child_sem);
    return 0;
}

