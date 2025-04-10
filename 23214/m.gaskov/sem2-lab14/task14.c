#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

sem_t sem_parent;
sem_t sem_child;

void print_error(int code, const char* msg, int should_exit) {
    if (code != 0) {
        perror(msg);
        if (should_exit) {
            exit(EXIT_FAILURE);
        }
    }
}

void *thread_function(void *arg) {
    for (int i = 0; i < 10; i++) {
        print_error(sem_wait(&sem_child), "sem_wait failure", 1);
        printf("Child thread: Line %d\n", i + 1);
        print_error(sem_post(&sem_parent), "sem_post failure", 1);
    }
    return NULL;
}

int main() {
    int code;
    pthread_t thread;
    print_error(sem_init(&sem_parent, 0, 1), "Failed to initialize sem_parent", 1);
    print_error(sem_init(&sem_child, 0, 0), "Failed to initialize sem_child", 1);
    if ((code = pthread_create(&thread, NULL, thread_function, NULL)) != 0) {
        fprintf(stderr, "Failed to create thread: %d.\n", code);
        return EXIT_FAILURE;
    }
    for (int i = 0; i < 10; i++) {
        print_error(sem_wait(&sem_parent), "sem_wait failure", 1);
        printf("Parent thread: Line %d\n", i + 1);
        print_error(sem_post(&sem_child), "sem_post failure", 1);
    }
    if ((code = pthread_join(thread, NULL)) != 0) {
        fprintf(stderr, "Failed to join thread: %d.\n", code);
        return EXIT_FAILURE;
    }
    print_error(sem_destroy(&sem_parent), "sem_destroy failed", 0);
    print_error(sem_destroy(&sem_child), "sem_destroy failed", 0);
    return EXIT_SUCCESS;
}
