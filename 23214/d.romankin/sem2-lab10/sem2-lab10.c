#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
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

int sleep_seconds = 0;

int main (int argn, char **argv) {
  int i = 0;
  int wrong_string = 0;
  if (argn == 2) {
    while(argv[1][i] != '\0') {
      if (argv[1][i] < '0' || argv[1][i] > '9') {
        wrong_string = 1;
        fprintf(stderr, "you entered wrong value, can't be converted\n");
        break;
      }
      i++;
    }
    if (wrong_string == 0) {
      sleep_seconds = atoi (argv[1]);
    }
  }
  if (pthread_mutex_init (&foodlock, NULL) != 0) {
    fprintf(stderr, "foodlock mutex init error\n");
    exit(EXIT_FAILURE);
  }
  for (i = 0; i < PHILO; i++) {
    if (pthread_mutex_init (&forks[i], NULL) != 0) {
      fprintf(stderr, "fork mutex %d init error\n", i);
      exit(EXIT_FAILURE);
    }
  }
  for (i = 0; i < PHILO; i++)
    if (pthread_create (&phils[i], NULL, philosopher, *(void**)&i) != 0) {
      fprintf(stderr, "thread %d creation error\n", i);
      exit(EXIT_FAILURE);
    }
  for (i = 0; i < PHILO; i++)
    if (pthread_join (phils[i], NULL)) {
      fprintf(stderr, "thread %d joining error\n", i);
      exit(EXIT_FAILURE);
    }
  if (pthread_mutex_destroy(&foodlock) != 0) {
    fprintf(stderr, "foodlock mutex destroy error\n");
    exit(EXIT_FAILURE);
  }
  for (i = 0; i < PHILO; i++) {
    if (pthread_mutex_destroy(&forks[i]) != 0) {
      fprintf(stderr, "fork mutex %d destroy error\n", i);
      exit(EXIT_FAILURE);
    }
  }
  return 0;
}

void* philosopher (void *num) {
  int id;
  int left_fork, right_fork, f;

  id = *(int*)&num;
  printf ("Philosopher %d sitting down to dinner.\n", id);
  right_fork = id;
  left_fork = id + 1;
 
  if (left_fork == PHILO)
    left_fork = 0;
 
  while (f = food_on_table ()) {

    if (id == 1)
      sleep (sleep_seconds);

    printf ("Philosopher %d: get dish %d.\n", id, f);
    if (left_fork < right_fork) {
      get_fork (id, left_fork, "left ");
      get_fork (id, right_fork, "right");
    }
    else {
      get_fork (id, right_fork, "right ");
      get_fork (id, left_fork, "left");
    }
    printf ("Philosopher %d: eating.\n", id);
    usleep (DELAY * (FOOD - f + 1));
    down_forks (left_fork, right_fork);
  }
  printf ("Philosopher %d is done eating.\n", id);
  return (NULL);
}

int food_on_table () {
  static int food = FOOD;
  int myfood;

  assert(pthread_mutex_lock (&foodlock) == 0);
  if (food > 0) {
    food--;
  }
  myfood = food;
  assert(pthread_mutex_unlock (&foodlock) == 0);
  return myfood;
}

void get_fork (int phil, int fork, char *hand) {
  assert(pthread_mutex_lock (&forks[fork]) == 0);
  printf ("Philosopher %d: got %s fork %d\n", phil, hand, fork);
}

void down_forks (int f1, int f2) {
  assert(pthread_mutex_unlock (&forks[f1]) == 0);
  assert(pthread_mutex_unlock (&forks[f2]) == 0);
}
