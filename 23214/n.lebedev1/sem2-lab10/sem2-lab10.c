/*
 * File:   din_phil.c
 * Author: nd159473 (Nickolay Dalmatov, Sun Microsystems)
 * adapted from http://developers.sun.com/sunstudio/downloads/ssx/tha/tha_using_deadlock.html
 *
 * Created on January 1, 1970, 9:53 AM
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#define PHILO 5
#define DELAY 30000
#define FOOD 50

pthread_mutex_t forks[PHILO];
pthread_t phils[PHILO];

void *philosopher(void *id);

int food_on_table();

void get_fork(int, int, char *);

void down_forks(int, int);

pthread_mutex_t foodlock;

int food_on_table() {
    static int food = FOOD;
    int myfood;
    assert(pthread_mutex_lock(&foodlock) == 0);
    if (food > 0) {
        food--;
    }
    myfood = food;
    assert(pthread_mutex_unlock(&foodlock) == 0);
    return myfood;
}

void get_fork(int phil, int fork, char *hand) {
    assert(pthread_mutex_lock(&forks[fork]) == 0);
    printf("Philosopher %d: got %s fork %d\n", phil, hand, fork);
}

void down_forks(int f1, int f2) {
    assert(pthread_mutex_unlock(&forks[f1]) == 0);
    assert(pthread_mutex_unlock(&forks[f2]) == 0);
}

void *philosopher(void *num) {
    int id;
    int left_fork, right_fork, f;

    id = (int) (intptr_t) num;
    printf("Philosopher %d sitting down to dinner.\n", id);
    right_fork = id;
    left_fork = (id + 1) % PHILO;

    while ((f = food_on_table()) != 0) {
        printf("Philosopher %d: get dish %d.\n", id, f);
        if (left_fork == 0) { // last philosopher
            get_fork(id, left_fork, "left ");
            get_fork(id, right_fork, "right");
        }
        else{
            get_fork(id, right_fork, "right");
            get_fork(id, left_fork, "left ");
        }
        printf("Philosopher %d: eating.\n", id);
        usleep(DELAY * (FOOD - f + 1));
        down_forks(left_fork, right_fork);
    }
    printf("Philosopher %d is done eating.\n", id);
    return (NULL);
}

int main() {
    int i, errCode;;
    if ((errCode = pthread_mutex_init(&foodlock, NULL)) != 0) {
        fprintf(stderr, "ERROR: Mutex foodlock initialization failed: %s\n", strerror(errCode));
        exit(-1);
    }
    for (i = 0; i < PHILO; i++)
        if ((errCode = pthread_mutex_init(&forks[i], NULL)) != 0) {
            fprintf(stderr, "ERROR: Mutex fork initialization failed: %s\n", strerror(errCode));
            exit(-1);
        }
    for (i = 0; i < PHILO; i++)
        if ((errCode = pthread_create(&phils[i], NULL, philosopher, (void *) (intptr_t) i)) != 0) {
            fprintf(stderr, "ERROR: Thread creation failed: %s\n", strerror(errCode));
            exit(1);
        }
    for (i = 0; i < PHILO; i++)
        pthread_join(phils[i], NULL);
    pthread_mutex_destroy(&foodlock);
    for (i = 0; i < PHILO; i++)
        pthread_mutex_destroy(&forks[i]);
    return 0;
}
