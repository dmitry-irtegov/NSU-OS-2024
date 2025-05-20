#include "stdlib.h"
#include "pthread.h"
#include "stdio.h"
#include "string.h"
#include "unistd.h"
#include <semaphore.h>

sem_t sem1, sem2;

void handler(char str[], int num) {
	char buf[256];
	strerror_r(num, buf, 256);
	fprintf(stderr, "%s error: %s", str, buf);
	exit(1);
}


void* thread_funk() {
	int res = 0;

	for (int i = 0; i < 10; i++) {
		res = sem_wait(&sem2);
		if (res) {
			perror("sem2 wait");
			exit(EXIT_FAILURE);
		}

		printf("Thread: %d\n", i);

		res = sem_post(&sem1);
		if (res) {
			perror("sem1 post");
			exit(EXIT_FAILURE);
		}
		
	}

	pthread_exit(NULL);
}

void init() {
	
	int res = 0;
	
	res = sem_init(&sem1, 0, 1);
	if (res) {
		perror("sem1 init error");
		exit(EXIT_FAILURE);
	}

	res = sem_init(&sem2, 0, 0);
	if (res) {
		perror("sem2 init error");
		exit(EXIT_FAILURE);
	}

}


void printMain() {
	int res = 0;

	for (int i = 0; i < 10; i++) {
		res = sem_wait(&sem1);
		if (res) {
			perror("sem1 wait");
			exit(EXIT_FAILURE);
		}

		printf("Main: %d\n", i);
		
		res = sem_post(&sem2);
		if (res) {
			perror("sem2 post");
			exit(EXIT_FAILURE);
		}
	}

	return;
}

void destroy() {
	int res = 0;

	res = sem_destroy(&sem1);
	if (res) {
		perror("sem1 destroy error");
		exit(EXIT_FAILURE);
	}


	res = sem_destroy(&sem2);
	if (res) {
		perror("sem2 destroy error");
		exit(EXIT_FAILURE);
	}

}

int main() {
	pthread_t thread;
	pthread_attr_t attr;

	int checkRes = 0;

	init();

	checkRes = pthread_attr_init(&attr);
	if (checkRes != 0) {
		handler("attr_init", checkRes);
	}

	checkRes = pthread_create(&thread, &attr, thread_funk, NULL);
	if (checkRes != 0) {
		handler("create", checkRes);
	}


	printMain();
	checkRes = pthread_join(thread, NULL);
	if (checkRes) handler("join", checkRes);

	checkRes = pthread_attr_destroy(&attr);
	if (checkRes != 0) {
		handler("destroy", checkRes);
	}


	exit(EXIT_SUCCESS);
}