#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t mutex;
pthread_cond_t cond;
int turn = 0;

void* thread_function(void* arg) {
    for (int i = 0; i < 10; i++) {
        if (pthread_mutex_lock(&mutex) != 0) {
            perror("Error locking mutex in child thread");
            exit(EXIT_FAILURE);
        }
        while (turn != 1) {
            if (pthread_cond_wait(&cond, &mutex) != 0) {
                perror("Error waiting on condition variable in child thread");
                exit(EXIT_FAILURE);
            }
        }
        printf("Child thread: line %d\n", i);
        turn = 0;
        if (pthread_cond_signal(&cond) != 0) {
            perror("Error signaling condition variable in child thread");
            exit(EXIT_FAILURE);
        }
        if (pthread_mutex_unlock(&mutex) != 0) {
            perror("Error unlocking mutex in child thread");
            exit(EXIT_FAILURE);
        }
    }
    return NULL;
}

int main() {
    pthread_t thread;
    pthread_mutexattr_t attr;
    if (pthread_mutexattr_init(&attr) != 0) {
        perror("Error initializing mutex attributes");
        exit(EXIT_FAILURE);
    }
    if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK) != 0) {
        perror("Error setting mutex type");
        exit(EXIT_FAILURE);
    }
    if (pthread_mutex_init(&mutex, &attr) != 0) {
        perror("Error initializing mutex");
        exit(EXIT_FAILURE);
    }
    pthread_mutexattr_destroy(&attr);
    if (pthread_cond_init(&cond, NULL) != 0) {
        perror("Error initializing condition variable");
        exit(EXIT_FAILURE);
    }
    int result = pthread_create(&thread, NULL, thread_function, NULL);
    if (result != 0) {
        perror("Error creating thread");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < 10; i++) {
        if (pthread_mutex_lock(&mutex) != 0) {
            perror("Error locking mutex in parent thread");
            exit(EXIT_FAILURE);
        }
        while (turn != 0) {
            if (pthread_cond_wait(&cond, &mutex) != 0) {
                perror("Error waiting on condition variable in parent thread");
                exit(EXIT_FAILURE);
            }
        }
        printf("Parent thread: line %d\n", i);
        turn = 1;
        if (pthread_cond_signal(&cond) != 0) {
            perror("Error signaling condition variable in parent thread");
            exit(EXIT_FAILURE);
        }
        if (pthread_mutex_unlock(&mutex) != 0) {
            perror("Error unlocking mutex in parent thread");
            exit(EXIT_FAILURE);
        }
    }
    pthread_join(thread, NULL);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
    return 0;
}
