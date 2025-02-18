#include "stdio.h"
#include "stdlib.h"
#include "pthread.h"
#include "string.h"

typedef struct data {
	char strs[4][20];
	int cnt;
} data;

data d[4];

void* func(void *param) {
	
	data* d = param;
	for (int i = 0; i < d->cnt; i++) {
		printf("%s\n", d->strs[i]);
	}

	pthread_exit(NULL);
}

void handler(char str[], int num, int number) {
	char buf[256];
	strerror_r(num, buf, 256);
	fprintf(stderr, "%s %d error: %s", str, number, buf);
	exit(EXIT_FAILURE);
}

void start(pthread_t* thr, pthread_attr_t* attr, int numb, data* d) {
	int res = 0;
	
	res = pthread_create(thr, attr, func, d);
	if (res != 0) {
		handler("create", res, numb);
	}
}

void join(pthread_t thread, int num) {
	int res = 0;

	res = pthread_join(thread, NULL);
	if (res != 0) {
		handler("join", res, num);
	}
}

char* strs1[] = { "a", "b", "c", "d" };
char* strs2[] = { "00", "01", "10", "11" };
char* strs3[] = { "1", "2", "3", "4" };
char* strs4[] = { "5", "25", "125", "625" };


void set() {
	d[0].cnt = d[1].cnt = d[2].cnt = d[3].cnt = 4;
	for (int i = 0; i < 4; i++) {
		strcpy(d[0].strs[i], strs1[i]);
		strcpy(d[1].strs[i], strs2[i]);
		strcpy(d[2].strs[i], strs3[i]);
		strcpy(d[3].strs[i], strs4[i]);
	}
}


int main() {
	pthread_t thread[4];
	pthread_attr_t attr;
	set();

	int res = pthread_attr_init(&attr);
	if (res != 0) {
		handler("init", res, 0);
	}


	for (int i = 0; i < 4; i++) {
		start(&thread[i], &attr, i + 1, &d[i]);
	}

	for (int i = 0; i < 4; i++) {
		join(thread[i], i + 1);
	}

	res = pthread_attr_destroy(&attr);
	if (res != 0) {
		handler("destroy", res, 0);
	}

	pthread_exit(NULL);
}