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
void get_fork (long long, long long, char *);
void down_forks (long , long long);
pthread_mutex_t foodlock;

int sleep_seconds = 0;

void threads_perror(char* text, int code) {
    fprintf(stderr, "%s: %s \n", text, strerror(code));
}

int
main (int argn,
      char **argv)
{
  int code = 0;

  if (argn == 2)
    sleep_seconds = atoi (argv[1]);

  if ((code = pthread_mutex_init(&foodlock, NULL)) != 0){
      threads_perror("pthread_mutex_init foodlock failed", code);
      exit(EXIT_FAILURE);
  }
  for (int i = 0; i < PHILO; i++)
    if ((code = pthread_mutex_init(&forks[i], NULL)) != 0) {
      threads_perror("pthread_mutex_init forks failed", code);
      exit(EXIT_FAILURE);
    }
  for (long long i = 0; i < PHILO; i++)
    if ((code = pthread_create(&phils[i], NULL, philosopher, (void*) i)) != 0) {
      threads_perror("pthread_create failed", code);
      exit(EXIT_FAILURE);
    }
  for (int i = 0; i < PHILO; i++)
    if ((code = pthread_join(phils[i], NULL)) != 0) {
      threads_perror("pthread_join failed", code);
      exit(EXIT_FAILURE);
    }
  exit(EXIT_SUCCESS);
}

void *
philosopher (void *num)
{
  int f;
  long long left_fork, right_fork, id;
  id = (long long)num;
  printf ("Philosopher %d sitting down to dinner.\n", id);
  right_fork = id;
  left_fork = id + 1;
  if (left_fork == PHILO)
    left_fork = 0;
  while (f = food_on_table ()) {
    printf ("Philosopher %d: get dish %d.\n", id, f);
    if (left_fork == 0){
      get_fork (id, left_fork, "left ");
      get_fork (id, right_fork, "right");
      printf ("Philosopher %d: eating.\n", id);
      usleep (DELAY * (FOOD - f + 1));
      down_forks (right_fork, left_fork);
    }
    else{
      get_fork (id, right_fork, "right");
      get_fork (id, left_fork, "left ");
      printf ("Philosopher %d: eating.\n", id);
      usleep (DELAY * (FOOD - f + 1));
      down_forks (left_fork, right_fork);
    }

  }
  printf ("Philosopher %d is done eating.\n", id);
  pthread_exit(NULL);
}

int
food_on_table ()
{
  static int food = FOOD;
  int myfood, code = 0;

  if ((code = pthread_mutex_lock(&foodlock)) != 0) {
    threads_perror("pthread_mutex_lock(foodlock) failed", code);
    exit(EXIT_FAILURE);
  }
  if (food > 0) {
    food--;
  }
  myfood = food;
  if ((code = pthread_mutex_unlock(&foodlock)) != 0) {
    threads_perror("pthread_mutex_unlock(foodlock) failed", code);
    exit(EXIT_FAILURE);
  }
  return myfood;
}

void
get_fork (long long phil,
          long long fork,
          char *hand)
{
  int code = 0;
  if ((code = pthread_mutex_lock(&forks[fork])) != 0) {
    threads_perror("pthread_mutex_lock(forks[fork]) failed", code);
    exit(EXIT_FAILURE);
  }
  printf ("Philosopher %d: got %s fork %d\n", phil, hand, fork);
}

void
down_forks (long long f1,
            long long f2)
{
  int code = 0;
  if ((code = pthread_mutex_unlock (&forks[f1])) != 0) {
    threads_perror("pthread_mutex_unlock(forks[max_id]) failed", code);
    exit(EXIT_FAILURE);
  }
  if ((code = pthread_mutex_unlock (&forks[f2])) != 0) {
    threads_perror("pthread_mutex_unlock(forks[max_id]) failed", code);
    exit(EXIT_FAILURE);
  }
}
