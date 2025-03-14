#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define PHILO 5
#define DELAY 30000
#define FOOD 50

pthread_mutex_t foodlock;
pthread_mutex_t forks[PHILO];
pthread_t phils[PHILO];

void* philosopher(void* id);
void* philosopherSwap(void* id);
void get_fork(int, int, char*);
void down_forks(int, int);
int food_on_table();

int main() {
    int i;

    pthread_mutex_init(&foodlock, NULL);
    for (i = 0; i < PHILO; i++) {
        pthread_mutex_init(&forks[i], NULL);
    }

    for (i = 0; i < PHILO-1; i++) {
        pthread_create(&phils[i], NULL, philosopher,(void *)i);
    }
    pthread_create(&phils[PHILO-1], NULL, philosopherSwap,(void *)(PHILO-1));

    for (i = 0; i < PHILO; i++) {
        pthread_join(phils[i], NULL);
    }
    return 0;
}

void* philosopher(void *num)
{
    int id = (int)num;
    int left_fork = id;
    int right_fork = (id + 1) % PHILO;
    int f;

    printf("Philosopher %d sitting down to dinner.\n", id);

    while (f = food_on_table()) {
        printf("Philosopher %d: get dish %d.\n", id, f);
        get_fork(id, left_fork, "left");
        get_fork(id, right_fork, "right");

        printf("Philosopher %d: eating.\n", id);
        usleep(DELAY *(FOOD - f + 1));
        down_forks(left_fork, right_fork);
    }
    printf("Philosopher %d is done eating.\n", id);
    return(NULL);
}


void* philosopherSwap(void *num)
{
    int id = (int)num;
    int left_fork = id;
    int right_fork = (id + 1) % PHILO;
    int f;

    printf("Philosopher %d sitting down to dinner.\n", id);
    left_fork = id;
    right_fork = (id + 1) % PHILO;

    while (f = food_on_table()) {
        printf("Philosopher %d: get dish %d.\n", id, f);
        get_fork(id, right_fork, "right");
        get_fork(id, left_fork, "left");

        printf("Philosopher %d: eating.\n", id);
        usleep(DELAY *(FOOD - f + 1));
        down_forks(left_fork, right_fork);
    }
    printf("Philosopher %d is done eating.\n", id);
    return(NULL);
}

int food_on_table() {
    static int food = FOOD;
    int myfood;

    pthread_mutex_lock(&foodlock);
    if (food > 0) {
        food--;
    }
    myfood = food;
    pthread_mutex_unlock(&foodlock);
    return myfood;
}

void get_fork(int phil,int fork, char *hand) {
    pthread_mutex_lock(&forks[fork]);
    printf("Philosopher %d: got %s fork %d\n", phil, hand, fork);
}

void down_forks(int f1, int f2) {
    pthread_mutex_unlock(&forks[f1]);
    pthread_mutex_unlock(&forks[f2]);
}
