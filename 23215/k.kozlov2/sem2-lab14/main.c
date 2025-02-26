#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>

sem_t sem_P, sem_C;

void *thread_body(void *param) {
    for(int i = 0; i < 20; i++) {
        sem_wait(&sem_C);
        printf("Child thread number %d\n", i);
        sem_post(&sem_P);
    }

    sem_destroy(&sem_C);
    sem_destroy(&sem_P);

    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    pthread_t thread;
    int code;

    sem_init(&sem_P, 0, 1);
    sem_init(&sem_C, 0, 0);

    code = pthread_create(&thread, NULL, thread_body, NULL);
    if (code != 0) {
        fprintf(stderr, "%s: creating thread: %s\n", argv[0], strerror(code));
        sem_destroy(&sem_C);
        sem_destroy(&sem_P);    
        exit(EXIT_FAILURE);
    }

    for(int i = 0; i < 20; i++) {
        sem_wait(&sem_P);
        printf("Parent thread number %d\n", i);
        sem_post(&sem_C);
    }

    pthread_exit(NULL);
}
