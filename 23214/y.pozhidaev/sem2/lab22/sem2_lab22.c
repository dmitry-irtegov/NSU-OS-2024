#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

#define PHILO 5
#define DELAY 30000
#define FOOD 50

pthread_mutex_t forks[PHILO];
pthread_mutex_t waiter;
pthread_mutex_t foodlock;
pthread_cond_t condition;


pthread_t phils[PHILO];


void *philosopher(void *id);

int food_on_table();

void get_forks(int, int, int);

void down_forks(int, int);


int sleep_seconds = 0;

int main(int argn,
         char **argv) {
    int i;
    int code;
    char buf[256];

    if (argn == 2)
        sleep_seconds = atoi(argv[1]);


    if ((code = (pthread_mutex_init(&foodlock, NULL))) != 0) {
        strerror_r(code, buf, sizeof(buf));
        fprintf(stderr, "%s: initializing mutex: %s\n", argv[0], buf);
        exit(1);
    }

    if ((code = (pthread_cond_init(&condition, NULL))) != 0) {
        strerror_r(code, buf, sizeof(buf));
        fprintf(stderr, "%s: initializing mutex: %s\n", argv[0], buf);
        exit(1);
    }

    if ((code = (pthread_mutex_init(&waiter, NULL))) != 0) {
        strerror_r(code, buf, sizeof(buf));
        fprintf(stderr, "%s: initializing mutex: %s\n", argv[0], buf);
        exit(1);
    }

    for (i = 0; i < PHILO; i++)
        if ((code = (pthread_mutex_init(&forks[i], NULL))) != 0) {
            strerror_r(code, buf, sizeof(buf));
            fprintf(stderr, "%s: initializing mutex %d: %s\n", argv[0], i, buf);
            exit(1);
        }
    for (i = 0; i < PHILO; i++)
        if ((code = (pthread_create(&phils[i], NULL, philosopher, (void *)(intptr_t)i))) != 0) {
            strerror_r(code, buf, sizeof(buf));
            fprintf(stderr, "%s: creating thread %d: %s\n", argv[0], i, buf);
            exit(1);
        }
    for (i = 0; i < PHILO; i++)
        if ((code = (pthread_join(phils[i], NULL)) != 0)) {
            strerror_r(code, buf, sizeof(buf));
            fprintf(stderr, "%s: joining thread %d: %s\n", argv[0], i, buf);
            exit(1);
        }
    if ((code = (pthread_mutex_destroy(&foodlock))) != 0) {
        strerror_r(code, buf, sizeof(buf));
        fprintf(stderr, "%s: destroying mutex: %s\n", argv[0], buf);
        exit(1);
    }
    if ((code = (pthread_mutex_destroy(&waiter))) != 0) {
        strerror_r(code, buf, sizeof(buf));
        fprintf(stderr, "%s: destroying mutex: %s\n", argv[0], buf);
        exit(1);
    }
    if ((code = (pthread_cond_destroy(&condition))) != 0) {
        strerror_r(code, buf, sizeof(buf));
        fprintf(stderr, "%s: destroying condition: %s\n", argv[0], buf);
        exit(1);
    }
    for (i = 0; i < PHILO; i++) {
        if ((code = (pthread_mutex_destroy(&forks[i]))) != 0) {
            strerror_r(code, buf, sizeof(buf));
            fprintf(stderr, "%s: joining thread %d: %s\n", argv[0], i, buf);
            exit(1);
        }
    }

    return 0;
}

void *philosopher(void *num) {
    int id;
    int left_fork, right_fork, f;

    id = (int) (intptr_t) num;
    printf("Philosopher %d sitting down to dinner.\n", id);
    right_fork = id;
    left_fork = id + 1;

    /* Wrap around the forks. */
    if (left_fork == PHILO)
        left_fork = 0;

    while ((f = food_on_table())) {

        printf("Philosopher %d: get dish %d.\n", id, f);

        get_forks(id, left_fork, right_fork);

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

    assert (pthread_mutex_lock(&foodlock) == 0);
    if (food > 0) {
        food--;
    }
    myfood = food;
    assert (pthread_mutex_unlock(&foodlock) == 0);
    return myfood;
}

void get_forks(int phil, int fork1, int fork2) {
    assert(pthread_mutex_lock(&waiter) == 0);
    int code;
    do {
        if(pthread_mutex_trylock(&forks[fork1]) == 0){
            printf("Philosopher %d: got fork %d\n", phil, fork1);

            if((code = pthread_mutex_trylock(&forks[fork2])) == 0) {
                printf("Philosopher %d: got fork %d\n", phil, fork2);
            } else {
                assert(pthread_mutex_unlock(&forks[fork1])==0);
                printf("Philosopher %d: put fork %d\n", phil, fork1);
            }
        }
        if (code) pthread_cond_wait(&condition, &waiter);
    } while(code);

    assert(pthread_mutex_unlock(&waiter) == 0);

}

void down_forks(int f1,
                int f2) {
    assert(pthread_mutex_lock(&waiter) == 0);

    assert(pthread_mutex_unlock(&forks[f1]) == 0);
    assert(pthread_mutex_unlock(&forks[f2]) == 0);
    pthread_cond_broadcast(&condition);

    assert(pthread_mutex_unlock(&waiter) == 0);
}

