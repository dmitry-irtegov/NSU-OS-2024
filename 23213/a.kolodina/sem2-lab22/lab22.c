#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define PHILO 5
#define DELAY 30000
#define FOOD 50

pthread_mutex_t forks[PHILO];
pthread_mutex_t getting_forks_mx;
pthread_cond_t getting_forks_cond;

pthread_t phils[PHILO];

void *philosopher(void *id);
int food_on_table();
void get_forks(int, int, int);
void down_forks(int, int);

pthread_mutex_t foodlock;

int main(void) {
    int i;
    int code;

    code = pthread_mutex_init(&foodlock, NULL);
    if (code != 0) {
        fprintf(stderr, "pthread_mutex_init error %s\n", strerror(code));
        exit(1);
    }

    for (i = 0; i < PHILO; i++) {
        code = pthread_mutex_init(&forks[i], NULL);
        if (code != 0) {
            fprintf(stderr, "pthread_mutex_init error %s\n", strerror(code));
            exit(1);
        }
    }

    code = pthread_mutex_init(&getting_forks_mx, NULL);
    if (code != 0) {
        fprintf(stderr, "pthread_mutex_init error %s\n", strerror(code));
        exit(1);
    }

    code = pthread_cond_init(&getting_forks_cond, NULL);
    if (code != 0) {
        fprintf(stderr, "pthread_cond_init error %s\n", strerror(code));
        exit(1);
    }

    for (i = 0; i < PHILO; i++) {
        code = pthread_create(&phils[i], NULL, philosopher, &i);
        if (code != 0) {
            fprintf(stderr, "pthread_create error %s\n", strerror(code));
            exit(1);
        }
    }
    for (i = 0; i < PHILO; i++)
        pthread_join(phils[i], NULL);

    return 0;
}

void *philosopher(void *num) {
    int id;
    int i, left_fork, right_fork, f;

    id = *(int*)num;
    printf("Philosopher %d sitting down to dinner.\n", id);
    right_fork = id;
    left_fork = id + 1;

    if (left_fork == PHILO)
        left_fork = 0;

    while ((f = food_on_table()) > 0) {
        printf("Philosopher %d: get dish %d.\n", id, f);
        get_forks(id, right_fork, left_fork);

        printf("Philosopher %d: eating.\n", id);
        usleep(DELAY * (FOOD - f + 1));
        down_forks(left_fork, right_fork);
    }
    printf("Philosopher %d is done eating.\n", id);
    return (NULL);
}

int food_on_table() {
    static int food = FOOD;
    int myfood;
    int code;
    code = pthread_mutex_lock(&foodlock);
    if (food > 0) {
        food--;
    }
    myfood = food;
    code = pthread_mutex_unlock(&foodlock);
    return myfood;
}

void get_forks(int phil, int fork1, int fork2) {
    int res;
    pthread_mutex_lock(&getting_forks_mx);
    do {
        res = pthread_mutex_trylock(&forks[fork1]);
        if (res == 0) {
            res = pthread_mutex_trylock(&forks[fork2]);
            if (res == 0) {
                printf("Philosopher %d got forks %d %d\n", phil, fork1, fork2);
            } else {
                pthread_mutex_unlock(&forks[fork1]);
            }
        }
        if (res != 0) {
            pthread_cond_wait(&getting_forks_cond, &getting_forks_mx);
        }
    } while (res);

     pthread_mutex_unlock(&getting_forks_mx);
}

void down_forks(int f1, int f2) {
    pthread_mutex_lock(&getting_forks_mx);

    pthread_mutex_unlock(&forks[f1]);
    pthread_mutex_unlock(&forks[f2]);
    pthread_cond_broadcast(&getting_forks_cond);
    
    pthread_mutex_unlock(&getting_forks_mx);
}
