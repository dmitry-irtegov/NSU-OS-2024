#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

pthread_cond_t cond;
pthread_mutex_t mutex;
int turn = 0;

void err_handler(char *msg, int errID) {
    fprintf(stderr, "%s: %s\n", msg, strerror(errID));
}

void* threadFunc() {
    int errID;
    for (int i = 1; i <= 10; i++) {
        if ((errID = pthread_mutex_lock(&mutex)) != 0) {
            err_handler("ERROR: failed in pthread_mutex_lock",  errID);
            exit(EXIT_FAILURE);
        }
        while (turn != 1) {
            pthread_cond_wait(&cond, &mutex);
        }
        printf("Child: %d\n", i);
        turn = 0;
        pthread_cond_signal(&cond);
        if ((errID = pthread_mutex_unlock(&mutex)) != 0) {
            err_handler("ERROR: failed in pthread_mutex_unlock",  errID);
            exit(EXIT_FAILURE);
        }
    }
    pthread_exit(NULL);
}

int main() {
    pthread_t thread;
    pthread_attr_t attr;
    pthread_mutexattr_t mutexattr;
    pthread_condattr_t condattr;
    int errID = 0;

    if ((errID = pthread_mutexattr_init(&mutexattr)) != 0) {
        err_handler("ERROR: failed in pthread_mutexattr_init", errID);
        exit(EXIT_FAILURE);
    }

    if ((errID = pthread_attr_init(&attr)) != 0) {
        err_handler("ERROR: failed in pthread_attr_init", errID);
        exit(EXIT_FAILURE);
    }

    if ((errID = pthread_condattr_init(&condattr)) != 0) {
        err_handler("ERROR: failed in pthread_condattr_init", errID);
        exit(EXIT_FAILURE);
    }

    if ((errID = pthread_mutex_init(&mutex, &mutexattr)) != 0) {
        err_handler("ERROR: failed in pthread_mutex_init", errID);
        exit(EXIT_FAILURE);
    }

    if ((errID = pthread_cond_init(&cond, &condattr)) != 0) {
        err_handler("ERROR: failed in pthread_cond_init", errID);
    }

    if ((errID = pthread_create(&thread, &attr, threadFunc, NULL)) != 0) {
        err_handler("ERROR: failed in pthread_create", errID);
        exit(EXIT_FAILURE);
    }

    for (int i = 1; i <= 10; i++) {
        if ((errID = pthread_mutex_lock(&mutex)) != 0) {
            err_handler("ERROR: failed in pthread_mutex_lock",  errID);
            exit(EXIT_FAILURE);
        }
        while (turn != 0) {
            pthread_cond_wait(&cond, &mutex);
        }
        printf("Parent: %d\n", i);
        turn = 1;
        pthread_cond_signal(&cond);
        if ((errID = pthread_mutex_unlock(&mutex)) != 0) {
            err_handler("ERROR: failed in pthread_mutex_unlock",  errID);
            exit(EXIT_FAILURE);
        }
    }

    pthread_join(thread, NULL);

    if ((errID = pthread_attr_destroy(&attr)) != 0) {
        err_handler("ERROR: failed in pthread_attr_destroy", errID);
        exit(EXIT_FAILURE);
    }

    if ((errID = pthread_condattr_destroy(&condattr)) != 0) {
        err_handler("ERROR: failed in pthread_condattr_destroy", errID);
        exit(EXIT_FAILURE);
    }

    if ((errID = pthread_mutexattr_destroy(&mutexattr)) != 0) {
        err_handler("ERROR: failed in pthread_mutexattr_destroy", errID);
        exit(EXIT_FAILURE);
    }

    if ((errID = pthread_mutex_destroy(&mutex)) != 0) {
        err_handler("ERROR: failed in pthread_mutex_destroy", errID);
        exit(EXIT_FAILURE);
    }

    if ((errID = pthread_cond_destroy(&cond)) != 0) {
        err_handler("ERROR: failed in pthread_cond_destroy", errID);
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}