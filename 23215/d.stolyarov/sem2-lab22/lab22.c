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

pthread_mutex_t forks[PHILO];
pthread_t phils[PHILO];
void *philosopher (void *id);
int food_on_table ();
void get_forks (int);
void down_forks (int);
pthread_mutex_t foodlock;
pthread_mutex_t forksLock;
pthread_cond_t cond;

int sleep_seconds = 0;

int
main ()
{
  long long i;

  pthread_mutex_init (&foodlock, NULL);
  pthread_mutex_init (&forksLock, NULL);
  for (i = 0; i < PHILO; i++)
    pthread_mutex_init (&forks[i], NULL);
  for (i = 0; i < PHILO; i++)
    pthread_create (&phils[i], NULL, philosopher, (void *)i);
  for (i = 0; i < PHILO; i++)
    pthread_join (phils[i], NULL);
  return 0;
}

void *
philosopher (void *num)
{
  long long id;
  int f;

  id = (long long)num;
  printf ("Philosopher %lld sitting down to dinner.\n", id);
 
  while ((f = food_on_table ()) != 0) {

    printf ("Philosopher %lld: get dish %d.\n", id, f);
    get_forks(id);
    printf ("Philosopher %lld: eating.\n", id);
    usleep (DELAY * (FOOD - f + 1));
    down_forks (id);
  }
  printf ("Philosopher %lld is done eating.\n", id);
  return (NULL);
}

int
food_on_table ()
{
  static int food = FOOD;
  int myfood;

  pthread_mutex_lock (&foodlock);
  if (food > 0) {
    food--;
  }
  myfood = food;
  pthread_mutex_unlock (&foodlock);
  return myfood;
}

void
get_forks (int id)
{
  int left_fork, right_fork;
  right_fork = id;
  left_fork = id + 1;
 
  /* Wrap around the forks. */
  if (left_fork == PHILO)
    left_fork = 0;

  int done = 0;
  pthread_mutex_lock (&forksLock);
  while(done == 0){
    if(!pthread_mutex_trylock(&forks[right_fork])){
      if(!pthread_mutex_trylock(&forks[left_fork])){
        done = 1;
      }
      else{
        pthread_mutex_unlock(&forks[right_fork]);
      }
    }

    if(done == 0){
      pthread_cond_wait(&cond, &forksLock);
    }
  }
  pthread_mutex_unlock(&forksLock);
  printf ("Philosopher %d: got forks %d, %d\n", id, right_fork, left_fork);
}

void
down_forks (int id)
{
  int left_fork, right_fork;
  right_fork = id;
  left_fork = id + 1;
 
  /* Wrap around the forks. */
  if (left_fork == PHILO)
    left_fork = 0;

  pthread_mutex_lock (&forksLock);
  pthread_mutex_unlock (&forks[right_fork]);
  pthread_mutex_unlock (&forks[left_fork]);
  pthread_cond_broadcast(&cond);
  pthread_mutex_unlock (&forksLock);
}
