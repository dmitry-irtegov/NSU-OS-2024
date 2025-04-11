#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define MUTEX_COUNT 3

pthread_mutex_t mutexes[MUTEX_COUNT];
bool printed = false;

void* ThreadRoutine(void* args){
    if (!printed){
        pthread_mutex_lock(&mutexes[0]);
    }
    int i;
    for(i = 0; i < 20; i++){
        pthread_mutex_lock(&mutexes[(i + 1) % MUTEX_COUNT]);
        printf("child\n");
        printed = true;
        pthread_mutex_unlock(&mutexes[i % MUTEX_COUNT]);
    }
    pthread_mutex_unlock(&mutexes[i % MUTEX_COUNT]);
}

int main(){
    pthread_mutexattr_t mattr;

    pthread_mutexattr_init(&mattr);
    if (pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_ERRORCHECK)) {
        perror("pthread_mutexattr_settype:");
    }

    for (int i = 0; i < MUTEX_COUNT; i++){
        pthread_mutex_init(&mutexes[i], &mattr);
    }

    pthread_t childThread;

    if (pthread_create(&childThread, NULL, ThreadRoutine, NULL)){
        perror("pthread_create:");
    }

    pthread_mutex_lock(&mutexes[MUTEX_COUNT-1]);

    while (!printed) {
        sched_yield();
    }
    int i;
    for( i = 0; i < 20; i++){
        pthread_mutex_lock(&mutexes[i % MUTEX_COUNT]);
        printf("father\n");
        pthread_mutex_unlock(&mutexes[(i + 2) % MUTEX_COUNT]);
    }

    pthread_mutex_unlock(&mutexes[(i - 1) % MUTEX_COUNT]);

    if (pthread_join(childThread, NULL) != 0) {
        for (int i = 0; i < MUTEX_COUNT; i++) {
            pthread_mutex_destroy(&mutexes[i]);
        }
        perror("failed to join thread.");
        exit(-1);
    }

    for (int i = 0; i < MUTEX_COUNT; i++) {
        pthread_mutex_destroy(&mutexes[i]);
    }

    exit(0);
}
