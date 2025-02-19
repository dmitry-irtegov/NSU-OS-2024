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
void get_fork (int, int, char *);
void down_forks (int, int);
pthread_mutex_t foodlock;

int
main (int argn,
      char **argv)
{
  int i;

  pthread_mutex_init (&foodlock, NULL);
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
  int id;
  int left_fork, right_fork, f;

  id = (int)num;
  printf ("Philosopher %d sitting down to dinner.\n", id);
  right_fork = id;
  left_fork = id + 1;
 
  /* Wrap around the forks. */
  if (left_fork == PHILO)
    left_fork = 0;
 
  while (f = food_on_table ()) {

    printf ("Philosopher %d: get dish %d.\n", id, f);
    if(id%2){
        get_fork (id, left_fork, "left ");
        get_fork (id, right_fork, "right");
    }
    else{
        get_fork (id, right_fork, "right");
        get_fork (id, left_fork, "left ");
    }

    printf ("Philosopher %d: eating.\n", id);
    usleep (DELAY * (FOOD - f + 1));
    down_forks (left_fork, right_fork);
  }
  printf ("Philosopher %d is done eating.\n", id);
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
get_fork (int phil,
          int fork,
          char *hand)
{
  pthread_mutex_lock (&forks[fork]);
  printf ("Philosopher %d: got %s fork %d\n", phil, hand, fork);
}

void
down_forks (int f1,
            int f2)
{
  pthread_mutex_unlock (&forks[f1]);
  pthread_mutex_unlock (&forks[f2]);
}
