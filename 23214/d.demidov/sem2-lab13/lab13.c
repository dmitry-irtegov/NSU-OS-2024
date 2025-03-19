#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

pthread_mutex_t mutex;
pthread_cond_t cond_parent;
pthread_cond_t cond_child;

int is_parent = 1;  

void* thread_body(void* param) {
    for (int i = 0; i < 10; i++) {
        pthread_mutex_lock(&mutex);
        while (is_parent != 0) {
            pthread_cond_wait(&cond_child, &mutex);
        }
        printf("Child: Line %d\n", i);
        is_parent = 1;
        pthread_cond_signal(&cond_parent); 
        pthread_mutex_unlock(&mutex); 
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    pthread_t thread;
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond_parent, NULL);
    pthread_cond_init(&cond_child, NULL);
    if (0 != pthread_create(&thread, NULL, thread_body, NULL)) {
        perror("Error creating thread");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < 10; i++) {
        pthread_mutex_lock(&mutex); 
        while (is_parent != 1) {
            pthread_cond_wait(&cond_parent, &mutex);
        }
        printf("Parent: Line %d\n", i);
        is_parent = 0;
        pthread_cond_signal(&cond_child);
        pthread_mutex_unlock(&mutex);
    }

    pthread_join(thread, NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_parent);
    pthread_cond_destroy(&cond_child);

    return 0;
}