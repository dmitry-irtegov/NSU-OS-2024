#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>


sem_t parent_sem, child_sem;

void wait_err_handling(sem_t* sem) {
    if (sem_wait(sem) == -1) {
        perror("Wait error");
        exit(-1);
    }
}

void post_err_handling(sem_t* sem) {
    if (sem_post(sem) == -1) {
        perror("Post error");
        exit(-1);
    }
}

void* child_thread(void* arg) {
    for (int i = 0; i < 10; i++) {
        wait_err_handling(&child_sem);
        printf("Child thread: Line %d\n", i + 1);
        post_err_handling(&parent_sem);
    }
    return NULL;
}

int main() {
    pthread_t thread;

    if (sem_init(&parent_sem, 0, 1)) {
        perror("Parent sem init error");
        exit(-1);
    }
    if (sem_init(&child_sem, 0, 0)) {
        perror("Child sem init error");
        sem_destroy(&parent_sem);
        exit(-1);
    }

    int err = pthread_create(&thread, NULL, child_thread, NULL);
    if (err != 0) {
        fprintf(stderr, "Create error: %s\n", strerror(err));
        sem_destroy(&parent_sem);
        sem_destroy(&child_sem);
        return 1;
    }

    for (int i = 0; i < 10; i++) {
        wait_err_handling(&parent_sem);
        printf("Parent thread: Line %d\n", i + 1);
        post_err_handling(&child_sem);
    }

    err = pthread_join(thread, NULL);
    if (err != 0) {
        fprintf(stderr, "Join error: %s\n", strerror(err));
    }
    sem_destroy(&parent_sem);
    sem_destroy(&child_sem);

    return 0;
}
