#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define NUMBER_OF_SEM 4
#define NUMBER_OF_THREAD 5

sem_t sems[NUMBER_OF_SEM];
pthread_t pthreads[NUMBER_OF_THREAD];
int counts[NUMBER_OF_THREAD];

void sig_handler() {
    int errCode;
    for (int i = 0; i < NUMBER_OF_THREAD; i++) {
        if ((errCode = pthread_cancel(pthreads[i])) != 0) {
            fprintf(stderr, "ERROR: Thread canceling failed: %s\n", strerror(errCode));
            exit(-1);
        }
    }
    printf("---------------------------------\n");
    printf("All threads canceled correctly\n");
    printf("---------------------------------\n");
}


void *make_A() {
    while (1) {
        sleep(1);
        sem_post(&sems[0]);
        counts[0]++;
        printf("Created %d unit A\n", counts[0]);
    }
}

void *make_B() {
    while (1) {
        sleep(2);
        sem_post(&sems[1]);
        counts[1]++;
        printf("Created %d unit B\n", counts[1]);
    }
}

void *make_C() {
    while (1) {
        sleep(3);
        sem_post(&sems[2]);
        counts[2]++;
        printf("Created %d unit C\n", counts[2]);
    }
}

void *make_Module() {
    while (1) {
        sem_wait(&sems[0]);
        sem_wait(&sems[1]);
        counts[3]++;
        printf("Created module %d from A and B\n", counts[3]);
        sem_post(&sems[3]);
    }
}

void *make_Widget() {
    while (1) {
        sem_wait(&sems[2]);
        sem_wait(&sems[3]);
        counts[4]++;
        printf("Created widget %d\n", counts[4]);
    }
}

int main() {
    int errCode;
    sigset(SIGINT, sig_handler);
    for (int i = 0; i < NUMBER_OF_SEM; i++) {
        if ((errCode = sem_init(&sems[i], 0, 0)) != 0) {
            fprintf(stderr, "ERROR: Unable to initialize %d semaphore: %s\n", i, strerror(errCode));
            exit(1);
        }
    }
    printf("---------------------------------\n");
    printf("Factory started\n");
    printf("---------------------------------\n");
    if ((errCode = pthread_create(&pthreads[0], NULL, make_A, NULL)) != 0) {
        fprintf(stderr, "ERROR: Thread creation failed: %s\n", strerror(errCode));
        exit(1);
    }
    if ((errCode = pthread_create(&pthreads[1], NULL, make_B, NULL)) != 0) {
        fprintf(stderr, "ERROR: Thread creation failed: %s\n", strerror(errCode));
        exit(1);
    }
    if ((errCode = pthread_create(&pthreads[2], NULL, make_C, NULL)) != 0) {
        fprintf(stderr, "ERROR: Thread creation failed: %s\n", strerror(errCode));
        exit(1);
    }
    if ((errCode = pthread_create(&pthreads[3], NULL, make_Module, NULL)) != 0) {
        fprintf(stderr, "ERROR: Thread creation failed: %s\n", strerror(errCode));
        exit(1);
    }
    if ((errCode = pthread_create(&pthreads[4], NULL, make_Widget, NULL)) != 0) {
        fprintf(stderr, "ERROR: Thread creation failed: %s\n", strerror(errCode));
        exit(1);
    }
    for (int i = 0; i < NUMBER_OF_THREAD; i++) {
        pthread_join(pthreads[i], NULL);
    }
    for (int i = 0; i < NUMBER_OF_SEM; i++) {
        sem_destroy(&sems[i]);
    }
    printf("---------------------------------\n");
    printf("Production:\nUnits A - %d\nUnits B - %d\nUnits C - %d\nModules - %d\nWidgets - %d\n", counts[0], counts[1],
           counts[2], counts[3], counts[4]);
    printf("---------------------------------\n");
    exit(0);
}
