#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

pthread_mutex_t  parent[2], child[2];

void* routine(void* arg) {
    pthread_mutex_lock(&parent[1]);
    for (int i = 0; i < 10; i++) {
        pthread_mutex_lock(&child[i % 2]);
        printf("Hello from %s thread!\n", (char*) arg);
        pthread_mutex_unlock(&child[i % 2]);
        pthread_mutex_lock(&parent[i % 2]);
        pthread_mutex_unlock(&parent[(i + 1) % 2]);
    }

    pthread_mutex_unlock(&parent[0]);
    pthread_mutex_unlock(&parent[1]);
    
    return NULL;
}

int main() {
    pthread_mutexattr_t mutex_attr;
    pthread_mutexattr_init(&mutex_attr);
    pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_ERRORCHECK);


    for (int i = 0; i < 2; i++) {
        pthread_mutex_init(&parent[i], &mutex_attr);
        pthread_mutex_init(&child[i], &mutex_attr);
    }
    
    pthread_mutexattr_destroy(&mutex_attr);
    
    pthread_mutex_lock(&child[0]);
   
    pthread_t th;
    pthread_create(&th, NULL, routine, "child");
    
    while(pthread_mutex_trylock(&parent[1]) != EBUSY) {
        pthread_mutex_unlock(&parent[1]);
    }

    for (int i = 0; i < 10; i++) {
        pthread_mutex_lock(&parent[i % 2]);
        printf("Hello from parent thread!\n");
        pthread_mutex_unlock(&parent[i % 2]);
        pthread_mutex_lock(&child[(i + 1) % 2]);
        pthread_mutex_unlock(&child[i % 2]);
    }

    pthread_mutex_unlock(&child[0]);
    pthread_mutex_unlock(&child[1]);

    pthread_join(th, NULL);
 
    for (int i = 0; i < 2; i++) {
        pthread_mutex_destroy(&child[i]);
        pthread_mutex_destroy(&parent[i]);
    }

    exit(EXIT_SUCCESS);
}
