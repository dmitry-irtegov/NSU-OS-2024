#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

sem_t sem_parent;
sem_t sem_child;

void* child_thread(void* arg) {
    for (int i = 0; i < 10; ++i) {
        sem_wait(&sem_child);
        printf("Child: %d\n", i);
        sem_post(&sem_parent); 
    }
    return NULL;
}

int main() {
    pthread_t child;
    sem_init(&sem_parent, 0, 1);
    sem_init(&sem_child, 0, 0);
    if (pthread_create(&child, NULL, child_thread, NULL) != 0) {
        perror("thread create failed");
        sem_destroy(&sem_child);
        sem_destroy(&sem_parent);
        exit(1);
    }

    for (int i = 0; i < 10; ++i) {
        sem_wait(&sem_parent);
        printf("Parent: %d\n", i);
        sem_post(&sem_child);
    }

    pthread_join(child, NULL);

    sem_destroy(&sem_parent);
    sem_destroy(&sem_child);

    exit(0);
}