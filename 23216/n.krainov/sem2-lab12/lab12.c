#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

pthread_mutex_t mutexA;
pthread_mutex_t mutexB;
pthread_mutex_t mutexC;

void my_perror(char* text, int code) {
    fprintf(stderr, "%s: %s \n", text, strerror(code));
}

void* threadFunc(void* ignored) {
    int err = 0;
    int count = 0;
    if ((err = pthread_mutex_lock(&mutexC))) {
        my_perror("pthread_mutex_lock", err);
        exit(EXIT_FAILURE);
    }

    while (count < 4) {
        if ((err = pthread_mutex_lock(&mutexA))) {
            my_perror("pthread_mutex_lock", err);
            exit(EXIT_FAILURE);
        }

        printf("child %d\n", count);
        
        if ((err = pthread_mutex_unlock(&mutexC))) {
            my_perror("pthread_mutex_unlock", err);
            exit(EXIT_FAILURE);
        }

        if ((err = pthread_mutex_lock(&mutexB))) {
            my_perror("pthread_mutex_lock", err);
            exit(EXIT_FAILURE);
        }

        if ((err = pthread_mutex_unlock(&mutexA))) {
            my_perror("pthread_mutex_unlock", err);
            exit(EXIT_FAILURE);
        }
        if ((err = pthread_mutex_lock(&mutexC))) {
            my_perror("pthread_mutex_lock", err);
            exit(EXIT_FAILURE);
        }

        if ((err = pthread_mutex_unlock(&mutexB))) {
            my_perror("pthread_mutex_unlock", err);
            exit(EXIT_FAILURE);
        }
        count++;
    }

    pthread_exit(NULL);
}

int main() {
    int err = 0;
    pthread_t thread;

    pthread_mutexattr_t attr;
    if ((err = pthread_mutexattr_init(&attr))) {
        my_perror("pthread_mutexattr_init", err);
        exit(EXIT_FAILURE);
    }

    if ((err = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK))) {
        my_perror("pthread_mutexattr_settype", err);
        exit(EXIT_FAILURE);
    }

    if ((err = pthread_mutex_init(&mutexA, &attr))) {
        my_perror("pthread_mutex_init", err);
        exit(EXIT_FAILURE);
    }

    if ((err = pthread_mutex_init(&mutexB, &attr))) {
        my_perror("pthread_mutex_init", err);
        exit(EXIT_FAILURE);
    }

    if ((err = pthread_mutex_init(&mutexC, &attr))) {
        my_perror("pthread_mutex_init", err);
        exit(EXIT_FAILURE);
    }

    pthread_mutexattr_destroy(&attr);

    if ((err = pthread_mutex_lock(&mutexA))) {
        my_perror("pthread_mutex_lock", err);
        exit(EXIT_FAILURE);
    }

    if ((err = pthread_create(&thread, NULL, threadFunc, NULL))) {
        my_perror("pthread_create", err);
        exit(EXIT_FAILURE);
    }
    
    int count = 0;
    while (count < 4) {
        printf("parent %d\n", count);
        if ((err = pthread_mutex_lock(&mutexB))) {
            my_perror("pthread_mutex_lock", err);
            exit(EXIT_FAILURE);
        }

        if ((err = pthread_mutex_unlock(&mutexA))) {
            my_perror("pthread_mutex_unlock", err);
            exit(EXIT_FAILURE);
        }

        if ((err = pthread_mutex_lock(&mutexC))) {
            my_perror("pthread_mutex_lock", err);
            exit(EXIT_FAILURE);
        }

        if ((err = pthread_mutex_unlock(&mutexB))) {
            my_perror("pthread_mutex_unlock", err);
            exit(EXIT_FAILURE);
        }

        if ((err = pthread_mutex_lock(&mutexA))) {
            my_perror("pthread_mutex_lock", err);
            exit(EXIT_FAILURE);
        }

        if ((err = pthread_mutex_unlock(&mutexC))) {
            my_perror("pthread_mutex_unlock", err);
            exit(EXIT_FAILURE);
        }
        count++;
    }

    exit(EXIT_SUCCESS);
}