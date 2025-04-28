#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <errno.h>
#include <assert.h> 

sem_t sem_main;
sem_t sem_thread;

void *print_messages(void *arg) {
    int i = 0;
    for (i; i < 10; i++) {
        assert(sem_wait(&sem_thread) == 0); 
        printf("[Thread] Message %d\n", i + 1);
        assert(sem_post(&sem_main) == 0); 
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    pthread_t thread;
    int code;

    if (sem_init(&sem_main, 0, 1) != 0) { 
        perror("sem_init sem_main");
        exit(1);
    }

    if (sem_init(&sem_thread, 0, 0) != 0) {
        perror("sem_init sem_thread");
        exit(1);
    }

    code = pthread_create(&thread, NULL, print_messages, NULL);
    if (code != 0) {
        char buf[256];
        strerror_r(code, buf, sizeof(buf));
        fprintf(stderr, "%s: creating thread: %s\n", argv[0], buf);
        exit(1);
    }
    int i = 0;
    for (i; i < 10; i++) {
        assert(sem_wait(&sem_main) == 0);
        printf("[Main] Message %d\n", i + 1);
        assert(sem_post(&sem_thread) == 0); 
    }

    pthread_join(thread, NULL);

    sem_destroy(&sem_main);
    sem_destroy(&sem_thread);

    pthread_exit(NULL);
}
