#include "stdlib.h"
#include "pthread.h"
#include "stdio.h"
#include "string.h"
#include "unistd.h"

pthread_mutex_t mutex;
pthread_cond_t cond;

int turn;

void handler(char str[], int num) {
	char buf[256];
	strerror_r(num, buf, 256);
	fprintf(stderr, "%s error: %s", str, buf);
	exit(1);
}


void* thread_funk() {
	int res = 0;

	for (int i = 0; i < 10; i++) {
		res = pthread_mutex_lock(&mutex);
		if (res) handler("thread mutex lock", res);

		if (turn == 0) {
			res = pthread_cond_wait(&cond, &mutex);
			if (res) handler("cond wait thread", res);
		}

		printf("Thread: %d\n", i);
		turn = 0;

		res = pthread_cond_signal(&cond);
		if (res) handler("thread signal", res);

		res = pthread_mutex_unlock(&mutex);
		if (res) handler("thread mutex unlock", res);
	}

	pthread_exit(NULL);
}

void init() {
	turn = 0;
	int res = 0;
	res = pthread_mutex_init(&mutex, NULL);
	if (res) handler("mutex1 init", res);

	res = pthread_cond_init(&cond, NULL);
	if (res) handler("cond init error", res);

}


void printMain() {
	int res = 0;

	for (int i = 0; i < 10; i++) {
		res = pthread_mutex_lock(&mutex);
		if (res) handler("main init mutex lock", res);

		if (turn == 1) {
			res = pthread_cond_wait(&cond, &mutex);
			if (res) handler("cond wait main", res);
		}

		printf("Main: %d\n", i);
		turn = 1;

		res = pthread_cond_signal(&cond);
		if (res) handler("main signal", res);

		res = pthread_mutex_unlock(&mutex);
		if (res) handler("main mutex unlock", res);
	}

	return;
}

void destroy() {
	int res = 0;

	res = pthread_mutex_destroy(&mutex);
	if (res) handler("mutex destroy", res);

	res = pthread_cond_destroy(&cond);
	if (res) handler("cond destroy", res);

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
