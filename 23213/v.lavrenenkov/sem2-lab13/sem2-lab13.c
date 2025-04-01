#include <pthread.h>
#include <stdio.h>
#include <string.h>

int turn = 1;
pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  cond=PTHREAD_COND_INITIALIZER;

void * print10_child(void* val) {
    for(int i = 0; i < 10; i++) {
        pthread_mutex_lock(&mutex);
        while(turn) {
            pthread_cond_wait(&cond, &mutex);
        }
        printf("CHILD\n");
        turn = 1;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

void * print10_parent() {
    for(int i = 0; i < 10; i++) {
        pthread_mutex_lock(&mutex);
        while(!turn) {
            pthread_cond_wait(&cond, &mutex);
        }
        printf("PARENT\n");
        turn = 0;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main() {
    pthread_t th;
    int err = 0;
    if ((err = pthread_create(&th, NULL, print10_child, NULL)) != 0) {
        fprintf(stderr, "Couldn`t open thread: %s \n", strerror(err));
        return 1;
    }
    print10_parent();
    pthread_join(th, NULL);
    return 0;
}
