#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

sem_t sem_one, sem_two;

void* thread_func(void* param) {
    for(int i = 0; i < 10; i++) {
        sem_wait(&sem_two);
        printf("%d:new thread\n", i);
        sem_post(&sem_one);
    }
    return NULL;
}

int main() {
    pthread_t thread;
    sem_init(&sem_one, 0, 1);
    sem_init(&sem_two, 0, 0);

    int code = pthread_create(&thread, NULL, thread_func, NULL);
    if (code != 0) {
        perror("creating thread");
        exit(1);
    }


    for(int i = 0; i < 10; i++) {
        sem_wait(&sem_one);
        printf("%d:main thread\n", i);
        sem_post(&sem_two);
    }
    pthread_exit(NULL);

}
