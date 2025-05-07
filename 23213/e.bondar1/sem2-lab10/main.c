#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define PHILO 5
#define DELAY 30000
#define THINKING_DELAY 20000
#define FOOD 50

pthread_mutex_t forks[PHILO];
pthread_t phils[PHILO];
pthread_mutex_t foodlock;

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

void down_forks (int f1, int f2) {
	pthread_mutex_unlock (&forks[f1]);
	pthread_mutex_unlock (&forks[f2]);
}

void * philosopher (void *num) {
	int id;
	int left_fork, right_fork, f;

	id = (int)num;
	printf ("Philosopher %d sitting down to dinner.\n", id);
	right_fork = id;
	left_fork = (id + 1) % PHILO;

	if (left_fork == PHILO) left_fork = 0;

	while ((f = food_on_table()) > 0) {
		if (id == PHILO - 1) {
			get_fork(id, left_fork, "left ");
			get_fork(id, right_fork, "right");
		} else {
			get_fork(id, right_fork, "right");
			get_fork(id, left_fork, "left ");
		}

		printf ("Philosopher %d: eating.\n", id);
		usleep (DELAY * (FOOD - f + 1));
		down_forks (left_fork, right_fork);
		printf ("Philosopher %d is done eating.\n", id);

		printf("Philosopher %d: thinking. \n", id);
		usleep(THINKING_DELAY);
		printf("Philosopher %d is done thinking.\n", id);
	}
	return (NULL);
}

int main () {
	int i;

	int ret = pthread_mutex_init(&foodlock, NULL);
	if (ret != 0) {
		fprintf(stderr, "Failed to initialize foodlock: %s\n", strerror(ret));
		exit(EXIT_FAILURE);
	}

	for (i = 0; i < PHILO; i++) {
		ret = pthread_mutex_init(&forks[i], NULL);
		if (ret != 0) {
			fprintf(stderr, "Failed to initialize fork: %s\n", strerror(ret));
			exit(EXIT_FAILURE);
		}
	}

	for (i = 0; i < PHILO; i++) {
		int ret = pthread_create (&phils[i], NULL, philosopher, (void *)i);
		if (ret != 0) {
			fprintf(stderr, "Failed to cancel thread: %s\n", strerror(ret));
			exit(EXIT_FAILURE);
		}
	}

	for (i = 0; i < PHILO; i++) {
		pthread_join (phils[i], NULL);
	}
	return 0;
}
