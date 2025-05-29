#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define PHILO 5
#define DELAY 30000
#define FOOD 50

pthread_mutex_t forks[PHILO]; // вилки
pthread_mutex_t foodlock;     // мьютекс для доступа к еде
pthread_mutex_t fork_mutex;   // дополнитльный мьютекс
pthread_cond_t cond_var;      // условная переменная для ожидания вилок
pthread_t phils[PHILO];       // потоки-философы

void down_forks(int f1, int f2)
{
    pthread_mutex_unlock(&forks[f1]);
    pthread_mutex_unlock(&forks[f2]);
    pthread_cond_broadcast(&cond_var); // оповещаем сразу всех философов
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

    while ((f = food_on_table()))
    {
        printf("Philosopher %d: get dish %d.\n", id, f);

        // при попытке взять вилку философ захватывает мутекс
        pthread_mutex_lock(&fork_mutex);

        // проверяем доступность обеих вилок
        for (;;)
        {
            if (pthread_mutex_trylock(&forks[left_fork]) == 0)
            {
                if (pthread_mutex_trylock(&forks[right_fork]) == 0)
                {
                    printf("Philosopher %d: got forks %d, %d\n", id, left_fork, right_fork);
                    break;
                }
                else
                {
                    pthread_mutex_unlock(&forks[left_fork]);
                }
            }

            pthread_cond_wait(&cond_var, &fork_mutex);
        }

        pthread_mutex_unlock(&fork_mutex);

        printf("Philosopher %d: eating.\n", id);
        usleep(DELAY * (FOOD - f + 1));

        pthread_mutex_lock(&fork_mutex);
        down_forks(left_fork, right_fork); // тут я и вызываю cond_broadcast
        pthread_mutex_unlock(&fork_mutex);
    }

    printf("Philosopher %d is done eating.\n", id);
    return (NULL);
}

int main(int argn, char **argv)
{
    pthread_mutex_init(&foodlock, NULL);
    pthread_mutex_init(&fork_mutex, NULL);
    pthread_cond_init(&cond_var, NULL);

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

    pthread_cond_destroy(&cond_var);

    return 0;
}