#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#define handle_error_en(en, msg) \
    do { \
        errno = en; \
        perror(msg); \
        exit(EXIT_FAILURE); \
    } while (0)

sem_t sem_parent;
sem_t sem_child;

void print_all_lines(char *string, int i) {
    printf("%s: number %d\n", string, i);
}

void *print_lines(void *arg) {
    for (int i = 0; i < 10; i++) {
        if (sem_wait(&sem_child) != 0) {
            perror("sem_wait");
            pthread_exit(0);
        }
        print_all_lines("Thread", i);
        if (sem_post(&sem_parent) != 0) {
            perror("sem_post");
            pthread_exit(0);
        }
    }
    pthread_exit(0);
}

int main() {
    pthread_t thread;
    int s;

    if (sem_init(&sem_parent, 0, 1) != 0) {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }

    if (sem_init(&sem_child, 0, 0) != 0) {
        perror("sem_init");
        sem_destroy(&sem_parent);
        exit(EXIT_FAILURE);
    }

    if ((s = pthread_create(&thread, NULL, print_lines, NULL)) != 0) {
        handle_error_en(s, "pthread_create");
        sem_destroy(&sem_parent);
        sem_destroy(&sem_child);
    }

    for (int i = 0; i < 10; i++) {
        if (sem_wait(&sem_parent) != 0) {
            perror("sem_wait");
            sem_destroy(&sem_parent);
            sem_destroy(&sem_child);
            exit(EXIT_FAILURE);
        }
        print_all_lines("Parent thread", i);
        if (sem_post(&sem_child) != 0) {
            perror("sem_post");
            sem_destroy(&sem_parent);
            sem_destroy(&sem_child);
            exit(EXIT_FAILURE);
        }
    }

    if ((s = pthread_join(thread, NULL)) != 0) {
        handle_error_en(s, "pthread_join");
    }

    if (sem_destroy(&sem_parent) != 0) {
        perror("sem_destroy");
        exit(EXIT_FAILURE);
    }

    if (sem_destroy(&sem_child) != 0) {
        perror("sem_destroy");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
