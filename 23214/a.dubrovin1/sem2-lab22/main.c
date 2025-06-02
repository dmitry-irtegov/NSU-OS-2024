#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
 
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
 
pthread_mutex_t waiter;
//имя forks уже занято(
void atomic_get_forks(int, int, int);
pthread_cond_t cond;

int main (int argc, char ** argv) {
    int i;
    int code;
    char buf[256];

    code = pthread_mutex_init (&foodlock, NULL);
    if (code != 0) {
        strerror_r(code, buf, sizeof(buf));
        fprintf(stderr, "%s: initializing mutex: %s\n", argv[0], buf);
        exit(1);
    }

    code = pthread_mutex_init (&waiter, NULL);
    if (code != 0) {
        strerror_r(code, buf, sizeof(buf));
        fprintf(stderr, "%s: initializing mutex: %s\n", argv[0], buf);
        exit(1);
    }

    code = pthread_cond_init (&cond, NULL);
    if (code != 0) {
        strerror_r(code, buf, sizeof(buf));
        fprintf(stderr, "%s: initializing condvar: %s\n", argv[0], buf);
        exit(1);
    }

    for (i = 0; i < PHILO; i++) {
        code = pthread_mutex_init (&forks[i], NULL);
        if (code != 0) {
            strerror_r(code, buf, sizeof(buf));
            fprintf(stderr, "%s: initializing mutex: %s\n", argv[0], buf);
            exit(1);
        }
    }
    for (i = 0; i < PHILO; i++) {
        code = pthread_create (&phils[i], NULL, philosopher, (void *)i);
        if (code != 0) {
            strerror_r(code, buf, sizeof(buf));
            fprintf(stderr, "%s: creating thread %d: %s\n", argv[0], i, buf);
            exit(1);
        }
    }

    for (i = 0; i < PHILO; i++) {
        pthread_join (phils[i], NULL);
    }

    return 0;
}
 
void* philosopher (void *num) {
   int id;
   int left_fork, right_fork, f;
 
   id = (int) num;
   printf ("Philosopher %d sitting down to dinner.\n", id);
   right_fork = id;
   left_fork = id + 1;
  
   /* Wrap around the forks. */
    if (left_fork == PHILO) {
        left_fork = 0;
    }
  
    while (f = food_on_table()) {
        printf ("Philosopher %d: get dish %d.\n", id, f);
        
        atomic_get_forks(id, left_fork, right_fork);
 
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
 
    pthread_mutex_lock (&foodlock);
    if (food > 0) {
        food--;
    }
    myfood = food;
    pthread_mutex_unlock (&foodlock);
    return myfood;
}
 
void get_fork (int phil, int fork, char *hand) {
    pthread_mutex_lock (&forks[fork]);
    printf ("Philosopher %d: got %s fork %d\n", phil, hand, fork);
}

void atomic_get_forks(int phil, int fork1, int fork2) {
    assert(pthread_mutex_lock(&waiter) == 0);
    int code = 1;
    while (code) {
        code = pthread_mutex_trylock(&forks[fork1]);
        if (code == 0){
            printf("Philosopher %d: got fork %d\n", phil, fork1);

            code = pthread_mutex_trylock(&forks[fork2]);
            if (code  == 0) {
                printf("Philosopher %d: got fork %d\n", phil, fork2);
            } else {
                assert(pthread_mutex_unlock(&forks[fork1])==0);
                printf("Philosopher %d: put fork %d back\n", phil, fork1);
            }
        }

        if (code) {
            pthread_cond_wait(&cond, &waiter);
        }
    }

    code = pthread_mutex_unlock(&waiter);
    assert(code == 0);
}

 
void down_forks (int f1, int f2) {
   int code;

   code = pthread_mutex_lock(&waiter);
   assert(code == 0);

   code = pthread_mutex_unlock(&forks[f1]);
   assert(code == 0);

   code = pthread_mutex_unlock(&forks[f2]);
   assert(code == 0);

   pthread_cond_broadcast(&cond);

   code = pthread_mutex_unlock(&waiter);
   assert(code == 0);
}
