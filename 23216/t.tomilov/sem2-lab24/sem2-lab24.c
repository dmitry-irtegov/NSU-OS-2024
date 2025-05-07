#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

volatile int num_wigets;

sem_t sems[4];

void err_handler(char *msg, int errID) {
    fprintf(stderr, "%s: %s\n", msg, strerror(errID));
}

void *make_a() {
    while (num_wigets != 0) {
        sleep(1);
        printf("Created A\n");
        sem_post(&sems[0]);
    }
    pthread_exit(NULL);
}

void *make_b() {
    while (num_wigets != 0) {
        sleep(2);
        printf("Created B\n");
        sem_post(&sems[1]);
    }
    pthread_exit(NULL);
}

void *make_c() {
    while (num_wigets != 0) {
        sleep(3);
        printf("Created C\n");
        sem_post(&sems[2]);
    }
    pthread_exit(NULL);
}

void *make_module() {
    while (num_wigets != 0) {
        sem_wait(&sems[0]);
        sem_wait(&sems[1]);
        printf("Created module\n");
        sem_post(&sems[3]);
    }
    pthread_exit(NULL);
}

void *make_widget() {
    while (num_wigets != 0) {
        sem_wait(&sems[2]);
        sem_wait(&sems[3]);
        printf("Created widget\n");
        num_wigets--;
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "ERROR: wrong number of arguments. Try %s <number_of_widgets>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    num_wigets = atoi(argv[1]);

    pthread_t threads[5];
    pthread_attr_t attr;

    int errID;

    for (int i = 0; i < 4; i++) {
        if ((errID = sem_init(&sems[i], 0, 0)) != 0) {
            err_handler("ERROR: failed to init semaphore", errID);
            exit(EXIT_FAILURE);
        }
    }

    if ((errID = pthread_attr_init(&attr)) != 0) {
        err_handler("ERROR: failed to init attr", errID);
        exit(EXIT_FAILURE);
    }

    if ((errID = pthread_create(&threads[0], &attr, make_a, 0)) != 0) {
        err_handler("ERROR: failed to create thread make_a", errID);
        exit(EXIT_FAILURE);
    }

    if ((errID = pthread_create(&threads[1], &attr, make_b, 0)) != 0) {
        err_handler("ERROR: failed to create thread make_b", errID);
        exit(EXIT_FAILURE);
    }

    if ((errID = pthread_create(&threads[2], &attr, make_c, 0)) != 0) {
        err_handler("ERROR: failed to create thread make_c", errID);
        exit(EXIT_FAILURE);
    }

    if ((errID = pthread_create(&threads[3], &attr, make_module, 0)) != 0) {
        err_handler("ERROR: failed to create thread make_module", errID);
        exit(EXIT_FAILURE);
    }

    if ((errID = pthread_create(&threads[4], &attr, make_widget, 0)) != 0) {
        err_handler("ERROR: failed to create thread make_widget", errID);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < 5; i++) {
        if ((errID = pthread_join(threads[i], NULL)) != 0) {
            err_handler("ERROR: failed to join thread", errID);
            exit(EXIT_FAILURE);
        }
    }

    if ((errID = pthread_attr_destroy(&attr)) != 0) {
        err_handler("ERROR: failed to destroy attr", errID);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < 4; i++) {
        if ((errID = sem_destroy(&sems[i])) != 0) {
            err_handler("ERROR: failed to destroy semaphore", errID);
            exit(EXIT_FAILURE);
        }
    }

    exit(EXIT_SUCCESS);
}
