#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#define STR_COUNT (10)

sem_t sem_child, sem_parent;

void* child(void* arg) {
    for (int i = 0; i < STR_COUNT; i++) {
        sem_wait(&sem_child);
        printf("child\n");
        sem_post(&sem_parent);
    }
    return NULL;
}


int main() {
    sem_init(&sem_child, 0, 0);
    sem_init(&sem_parent, 0, 1);

    pthread_t th;
    pthread_create(&th, NULL, child, NULL);

    for (int i = 0; i < STR_COUNT; i++) {
        sem_wait(&sem_parent);
        printf("parent\n");
        sem_post(&sem_child);
    }

    pthread_join(th, NULL);

    sem_destroy(&sem_child);
    sem_destroy(&sem_parent);

    exit(EXIT_SUCCESS);
}
