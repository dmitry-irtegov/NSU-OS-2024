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

#define PHILO 5
#define DELAY 30000
#define FOOD 50

pthread_mutex_t getting_forks_mx;
pthread_cond_t getting_forks_cond;

pthread_mutex_t forksInProgress;
pthread_mutex_t forks[PHILO];
pthread_t phils[PHILO];
void *philosopher(void *id);
int food_on_table();
void get_forks(int phil, int fork1, int fork2);
void down_forks(int, int);
pthread_mutex_t foodlock;

int sleep_seconds = 0;

int main(int argn, char **argv) {
  long long i;
  pthread_mutex_init(&forksInProgress, NULL);

  if (argn == 2)
    sleep_seconds = atoi(argv[1]);

  pthread_mutex_init(&foodlock, NULL);
  for (i = 0; i < PHILO; i++)
    pthread_mutex_init(&forks[i], NULL);
  for (i = 0; i < PHILO; i++)
    pthread_create(&phils[i], NULL, philosopher, (void *)i);
  for (i = 0; i < PHILO; i++)
    pthread_join(phils[i], NULL);
  return 0;
}

void *philosopher(void *num) {
  int id;
  int left_fork, right_fork, f;

  long long tmp = (long long)num;
  id = (int)tmp;
  printf("Philosopher %d sitting down to dinner.\n", id);
  right_fork = id;
  left_fork = id + 1;

  /* Wrap around the forks. */
  if (left_fork == PHILO)
    left_fork = 0;

  while ((f = food_on_table())) {

    /* Thanks to philosophers #1 who would like to
     * take a nap before picking up the forks, the other
     * philosophers may be able to eat their dishes and
     * not deadlock.
     */
    if (id == 1)
      sleep(sleep_seconds);
    
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

    pthread_mutex_trylock(&foodlock);
    if (food > 0) {
      food--;
    }
    myfood = food;
    pthread_mutex_unlock(&foodlock);
    return myfood;
  
}

void get_forks(int phil, int fork1, int fork2) {
  int res;
  pthread_mutex_lock(&getting_forks_mx);
  do {
    if ((res=pthread_mutex_trylock(&forks[fork1])) == 0) {
      res=pthread_mutex_trylock(&forks[fork2]);
      if (res)  {
        pthread_mutex_unlock(&forks[fork1]);
      }
    }
    if (res) pthread_cond_wait(&getting_forks_cond, &getting_forks_mx);
  } while(res);
  printf("Philosopher %d got forks %d and %d", phil, fork1, fork2);
  pthread_mutex_unlock(&getting_forks_mx);
}

void down_forks(int f1, int f2) {
  pthread_mutex_lock(&getting_forks_mx);
  pthread_mutex_unlock (&forks[f1]);
  pthread_mutex_unlock (&forks[f2]);
  pthread_cond_broadcast(&getting_forks_cond);
  pthread_mutex_unlock(&getting_forks_mx);
}

