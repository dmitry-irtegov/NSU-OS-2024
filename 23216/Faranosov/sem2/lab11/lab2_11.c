#include "stdlib.h"
#include "pthread.h"
#include "stdio.h"
#include "string.h"
#include "unistd.h"

pthread_mutex_t mutex1;
pthread_mutex_t mutex2;
pthread_mutex_t mutex3;
pthread_barrier_t barr;


void handler(char str[], int num) {
	char buf[256];
	strerror_r(num, buf, 256);
	fprintf(stderr, "%s error: %s", str, buf);
	exit(1);
}


void* thread_funk() {
	int num = 0;
	int res = 0;


	res = pthread_mutex_lock(&mutex3);
	if (res) handler("son mutex3 lock", res);

	res = pthread_barrier_wait(&barr);
	if (res) handler("barr son", res);

	while (num < 10) {
		res = pthread_mutex_lock(&mutex1);
		if (res) handler("son mutex1 lock", res);

		printf("Son: %d\n", num);
		num++;

		res = pthread_mutex_unlock(&mutex3);
		if (res) handler("son mutex3 unlock", res);

		res = pthread_mutex_lock(&mutex2);
		if (res) handler("son mutex2 lock", res);

		if (num < 10) printf("Son: %d\n", num);
		num++;

		res = pthread_mutex_unlock(&mutex1);
		if (res) handler("son mutex1 unlock", res);
		
		res = pthread_mutex_lock(&mutex3);
		if (res) handler("son mutex3 lock", res);

		if (num < 10) printf("Son: %d\n", num);
		num++;

		res = pthread_mutex_unlock(&mutex2);
		if (res) handler("sin mutex2 unlock", res);
	}

	res = pthread_mutex_unlock(&mutex3);
	if (res) handler("son mutex3 unlock", res);

	pthread_exit(NULL);
}

void init() {
	int res = 0;
	res = pthread_mutex_init(&mutex1, NULL);
	if (res) handler("mutex1 init", res);

	res = pthread_mutex_init(&mutex2, NULL);
	if (res) handler("mutex2 init", res);

	res = pthread_mutex_init(&mutex3, NULL);
	if (res) handler("mutex3 init", res);

	res = pthread_barrier_init(&barr, NULL, 2);
	if (res)handler("barr init", res);

	res = pthread_mutex_lock(&mutex1);
	if (res) handler("main init mutex1 lock", res);
}


void printMain() {
	int num = 0;
	int res = 0;

	while (num < 10) {
		res = pthread_mutex_lock(&mutex2);
		if (res) handler("main mutex2 lock", res);

		printf("Main: %d\n", num);
		num++;

		res = pthread_mutex_unlock(&mutex1);
		if (res) handler("main mutex1 unlock", res);

		res = pthread_mutex_lock(&mutex3);
		if (res) handler("main mutex3 lock", res);

		if (num < 10) printf("Main: %d\n", num);
		num++;

		res = pthread_mutex_unlock(&mutex2);
		if (res) handler("main mutex2 unlock", res);

		res = pthread_mutex_lock(&mutex1);
		if (res) handler("main mutex1 lock", res);

		if (num < 10) printf("Main: %d\n", num);
		num++;
		
		res = pthread_mutex_unlock(&mutex3);
		if (res) handler("main mutex3 unlock", res);
	}

	res = pthread_mutex_unlock(&mutex1);
	if (res) handler("main mutex1 unlock", res);
	

	return;
}

void destroy() {
	int res = 0;

	res = pthread_mutex_destroy(&mutex1);
	if (res) handler("mutex1 destroy", res);

	res = pthread_mutex_destroy(&mutex2);
	if (res) handler("mutex2 destroy", res);

	res = pthread_mutex_destroy(&mutex3);
	if (res) handler("mutex3 destroy", res);

	res = pthread_barrier_destroy(&barr);
	if (res) handler("barr destroy", res);

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

	checkRes = pthread_barrier_wait(&barr);
	if (checkRes != PTHREAD_BARRIER_SERIAL_THREAD && checkRes) handler("barr main", checkRes);

	checkRes = pthread_attr_destroy(&attr);
	if (checkRes != 0) {
		handler("destroy", checkRes);
	}


	printMain();

	checkRes = pthread_join(thread, NULL);
	if (checkRes) handler("join", checkRes);

	exit(EXIT_SUCCESS);
}
