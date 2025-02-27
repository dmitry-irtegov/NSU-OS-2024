#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <assert.h> 

sem_t sem_parent;
sem_t sem_child;

void *thread_function(void *arg) {
    for (int i = 0; i < 10; i++) {
        assert(sem_wait(&sem_child) == 0);
        printf("Child thread: %d  :-)\n", i + 1);
        assert(sem_post(&sem_parent) == 0);
    }
    pthread_exit(NULL);
}

int main() {
    pthread_t thread;
    int errCode;

    if (sem_init(&sem_parent, 0, 1) != 0) {
        perror("Parent sem init failed");
        exit(1);
    }
    if (sem_init(&sem_child, 0, 0) != 0) {
        perror("Child sem init failed");
        sem_destroy(&sem_parent);
        exit(1);
    }

    if ((errCode = pthread_create(&thread, NULL, thread_function, NULL)) != 0) {
        char* buf = strerror(errCode);
        fprintf(stderr, "Failed to create thread: %s\n", buf);
        exit(1);
    }

    for (int i = 0; i < 10; i++) {
        assert(sem_wait(&sem_parent) == 0);
        printf("Parent thread: %d  :-)\n", i + 1);
        assert(sem_post(&sem_child) == 0);
    }

    pthread_join(thread, NULL);

    sem_destroy(&sem_parent);
    sem_destroy(&sem_child);

    exit(0);
}
