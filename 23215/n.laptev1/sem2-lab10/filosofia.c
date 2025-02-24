#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define PHILO 5
#define DELAY 30000
#define FOOD 50

pthread_mutex_t forks[PHILO];//forks[i] - вилка.
pthread_mutex_t foodlock;
pthread_t phils[PHILO];//phils[i] - какой-то филосов в виде отдельного потока

//try to take a fork
void get_fork(int phil, int fork, char *hand)
{
    pthread_mutex_lock(&forks[fork]);
    printf("Philosopher %d: got %s fork %d\n", phil, hand, fork);
}

//put doan a fork.
void down_forks(int f1, int f2)
{
    pthread_mutex_unlock(&forks[f1]);
    pthread_mutex_unlock(&forks[f2]);
}

int food_on_table()
{
    static int food = FOOD;
    int myfood;

    pthread_mutex_lock(&foodlock);
    if (food > 0)
    {
        food--;
    }
    myfood = food;
    pthread_mutex_unlock(&foodlock);
    return myfood;
}

void *philosopher(void *num)
{
    int id;
    int left_fork, right_fork, f;

    id = (int)(intptr_t)num;
    printf("Philosopher %d sitting down to dinner.\n", id);
    right_fork = id;
    left_fork = id + 1;

    if (left_fork == PHILO)
        left_fork = 0;

    int first_fork, second_fork;
    char *first_hand, *second_hand;

    //философы всегда берут вилку с наименьшим номером.
    if (right_fork < left_fork)
    {
        first_fork = right_fork;
        second_fork = left_fork;
        first_hand = "right";
        second_hand = "left ";
    }
    else
    {
        first_fork = left_fork;
        second_fork = right_fork;
        first_hand = "left ";
        second_hand = "right";
    }

    while ((f = food_on_table()))
    {

        printf("Philosopher %d: get dish %d.\n", id, f);
        get_fork(id, first_fork, first_hand);
        get_fork(id, second_fork, second_hand);

        printf("Philosopher %d: eating.\n", id);
        usleep(DELAY * (FOOD - f + 1));
        down_forks(first_fork, second_fork);
    }
    printf("Philosopher %d is done eating.\n", id);
    return (NULL);
}

int main(int argn, char **argv)
{
    pthread_mutex_init(&foodlock, NULL);

    for (int i = 0; i < PHILO; i++)
    {
        pthread_mutex_init(&forks[i], NULL);
    }

    for (int i = 0; i < PHILO; i++)
    {
        pthread_create(&phils[i], NULL, philosopher, (void *)(intptr_t)i);
    }

    for (int i = 0; i < PHILO; i++)
    {
        pthread_join(phils[i], NULL);
    }
    return 0;
}
