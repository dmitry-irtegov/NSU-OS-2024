#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void print_error(int code, char* error, int should_exit) {
    if (code != 0) {
        fprintf(stderr, error, code);
        if (should_exit) {
            exit(EXIT_FAILURE);
        }
    }
}

pthread_mutex_t mutex1;
pthread_mutex_t mutex2;
pthread_mutex_t mutex3;

void go_to_state2() {
    print_error(pthread_mutex_unlock(&mutex2), "Failed to unlock mutex2: %d\n", 1);
    print_error(pthread_mutex_lock(&mutex3), "Failed to lock mutex3: %d\n", 1);
    print_error(pthread_mutex_unlock(&mutex1), "Failed to unlock mutex1: %d\n", 1);
}

void go_to_state1() {
    print_error(pthread_mutex_lock(&mutex2), "Failed to lock mutex2: %d\n", 1);
    print_error(pthread_mutex_unlock(&mutex3), "Failed to unlock mutex3: %d\n", 1);
    print_error(pthread_mutex_lock(&mutex1), "Failed to lock mutex1: %d\n", 1);
}

void *thread_function(void *arg) {
    print_error(pthread_mutex_lock(&mutex3), "Failed to lock mutex3: %d\n", 1);
    for (int i = 0; i < 10; i++) {
        go_to_state1();
        printf("Child thread: Line %d\n", i + 1);
        go_to_state2();
    }
    print_error(pthread_mutex_unlock(&mutex3), "Failed to unlock mutex3: %d\n", 1);
    return NULL;
}

int main() {
    pthread_t thread;

    pthread_mutexattr_t attr;
    print_error(pthread_mutexattr_init(&attr), "Failed to init mutex attr: %d\n", 1);
    print_error(pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK), "Failed to set mutex type: %d\n", 1);
    print_error(pthread_mutex_init(&mutex1, &attr), "Failed to init mutex1: %d\n", 1);
    print_error(pthread_mutex_init(&mutex2, &attr), "Failed to init mutex2: %d\n", 1);
    print_error(pthread_mutex_init(&mutex3, &attr), "Failed to init mutex3: %d\n", 1);
    print_error(pthread_mutexattr_destroy(&attr), "Failed to destroy mutex attr: %d\n", 0);

    print_error(pthread_mutex_lock(&mutex1), "Failed to lock mutex1: %d\n", 1);
    print_error(pthread_mutex_lock(&mutex2), "Failed to lock mutex2: %d\n", 1);

    print_error(pthread_create(&thread, NULL, thread_function, NULL), "Failed to create thread: %d.\n", 1);

    sleep(1);

    for (int i = 0; i < 10; i++) {
        printf("Parent thread: Line %d\n", i + 1);
        go_to_state2();
        go_to_state1();
    }

    print_error(pthread_join(thread, NULL), "Failed to join thread: %d.\n", 1);

    print_error(pthread_mutex_unlock(&mutex1), "Failed to unlock mutex1: %d\n", 1);
    print_error(pthread_mutex_unlock(&mutex2), "Failed to unlock mutex2: %d\n", 1);

    print_error(pthread_mutex_destroy(&mutex1), "Failed to destroy mutex1: %d\n", 0);
    print_error(pthread_mutex_destroy(&mutex2), "Failed to destroy mutex2: %d\n", 0);
    print_error(pthread_mutex_destroy(&mutex3), "Failed to destroy mutex3: %d\n", 0);

    return EXIT_SUCCESS;
}
