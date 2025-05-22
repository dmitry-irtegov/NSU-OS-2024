#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <errno.h>

#define WIDGETS_NEEDED 5

sem_t sA, sB, sC, sModule, sWidget;

void print_error(int code, const char* msg, int should_exit) {
    if (code != 0) {
        perror(msg);
        if (should_exit) {
            exit(EXIT_FAILURE);
        }
    }
}

void* A_creator(void* arg) {
    for (int i = 0; i < WIDGETS_NEEDED; ++i) {
        sleep(1);
        printf("Detail A created (%d/%d)\n", i + 1, WIDGETS_NEEDED);
        print_error(sem_post(&sA), "Failed to post semaphore sA", 1);
    }
    return NULL;
}

void* B_creator(void* arg) {
    for (int i = 0; i < WIDGETS_NEEDED; ++i) {
        sleep(2);
        printf("Detail B created (%d/%d)\n", i + 1, WIDGETS_NEEDED);
        print_error(sem_post(&sB), "Failed to post semaphore sB", 1);
    }
    return NULL;
}

void* C_creator(void* arg) {
    for (int i = 0; i < WIDGETS_NEEDED; ++i) {
        sleep(3);
        printf("Detail C created (%d/%d)\n", i + 1, WIDGETS_NEEDED);
        print_error(sem_post(&sC), "Failed to post semaphore sC", 1);
    }
    return NULL;
}

void* Module_creator(void* arg) {
    for (int i = 0; i < WIDGETS_NEEDED; ++i) {
        print_error(sem_wait(&sA), "Failed to wait semaphore sA", 1);
        print_error(sem_wait(&sB), "Failed to wait semaphore sB", 1);
        printf("Module created (%d/%d)\n", i + 1, WIDGETS_NEEDED);
        print_error(sem_post(&sModule), "Failed to post semaphore sModule", 1);
    }
}

void* Widget_creator(void* arg) {
    for (int i = 0; i < WIDGETS_NEEDED; ++i) {
        print_error(sem_wait(&sModule), "Failed to wait semaphore sModule", 1);
        print_error(sem_wait(&sC), "Failed to wait semaphore sC", 1);
        printf("Widget created (%d/%d)\n", i + 1, WIDGETS_NEEDED);
        print_error(sem_post(&sWidget), "Failed to post semaphore sWidget", 1);
    }
    return NULL;
}

int main() {
    print_error(sem_init(&sA, 0, 0), "Failed to initialize semaphore sA", 1);
    print_error(sem_init(&sB, 0, 0), "Failed to initialize semaphore sB", 1);
    print_error(sem_init(&sC, 0, 0), "Failed to initialize semaphore sC", 1);
    print_error(sem_init(&sModule, 0, 0), "Failed to initialize semaphore sModule", 1);
    print_error(sem_init(&sWidget, 0, 0), "Failed to initialize semaphore sWidget", 1);

    pthread_t threads[5];
    int code;
    if ((code = pthread_create(&threads[0], NULL, A_creator, NULL)) != 0) {
        fprintf(stderr, "Failed to create thread: %d.\n", code);
        return EXIT_FAILURE;
    }
    if ((code = pthread_create(&threads[1], NULL, B_creator, NULL)) != 0) {
        fprintf(stderr, "Failed to create thread: %d.\n", code);
        return EXIT_FAILURE;
    }
    if ((code = pthread_create(&threads[2], NULL, C_creator, NULL)) != 0) {
        fprintf(stderr, "Failed to create thread: %d.\n", code);
        return EXIT_FAILURE;
    }
    if ((code = pthread_create(&threads[3], NULL, Module_creator, NULL)) != 0) {
        fprintf(stderr, "Failed to create thread: %d.\n", code);
        return EXIT_FAILURE;
    }
    if ((code = pthread_create(&threads[4], NULL, Widget_creator, NULL)) != 0) {
        fprintf(stderr, "Failed to create thread: %d.\n", code);
        return EXIT_FAILURE;
    }

    for (int i = 0; i < WIDGETS_NEEDED; i++) {
        print_error(sem_wait(&sWidget), "Failed to wait on semaphore sWidget", 1);
        printf("Widget stored (%d/%d)\n", i + 1, WIDGETS_NEEDED);
    }

    for (int i = 0; i < 5; i++) {
        if ((code = pthread_join(threads[i], NULL)) != 0) {
            fprintf(stderr, "Failed to join thread: %d.\n", code);
        }
    }

    print_error(sem_destroy(&sA), "Failed to destroy semaphore sA", 0);
    print_error(sem_destroy(&sB), "Failed to destroy semaphore sB", 0);
    print_error(sem_destroy(&sC), "Failed to destroy semaphore sC", 0);
    print_error(sem_destroy(&sModule), "Failed to destroy semaphore sModule", 0);
    print_error(sem_destroy(&sWidget), "Failed to destroy semaphore sWidget", 0);
    return EXIT_SUCCESS;
}

