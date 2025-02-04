#include "stdio.h"
#include "stdlib.h"
#include "pthread.h"
#include "string.h"

typedef struct data {
	char strs[4][20];
	int cnt;
} data;

data d1, d2, d3, d4;

void* func(void *param) {
	
	data* d = param;
	for (int i = 0; i < d->cnt; i++) {
		printf("%s\n", d->strs[i]);
	}

	pthread_exit(NULL);
}

void start(pthread_t* thr, int numb, data* d) {
	int res = 0;
	res = pthread_create(thr, NULL, func, d);
	if (res != 0) {
		char buf[256];
		strerror_r(res, buf, 256);
		fprintf(stderr, "create %d error: %s", numb, buf);
		exit(EXIT_FAILURE);
	}

	res = pthread_detach(*thr);
	if (res != 0) {
		char buf[256];
		strerror_r(res, buf, 256);
		fprintf(stderr, "detach %d error: %s", numb, buf);
		exit(EXIT_FAILURE);
	}
}

int main() {
	pthread_t thread1, thread2, thread3, thread4;

	d1.cnt = 4;
	strcpy(d1.strs[0], "a");
	strcpy(d1.strs[1], "b");
	strcpy(d1.strs[2], "c");
	strcpy(d1.strs[3], "d");
	
	d2.cnt = 4;
	strcpy(d2.strs[0], "00");
	strcpy(d2.strs[1], "01");
	strcpy(d2.strs[2], "10");
	strcpy(d2.strs[3], "11");

	d3.cnt = 4;
	strcpy(d3.strs[0], "1");
	strcpy(d3.strs[1], "2");
	strcpy(d3.strs[2], "3");
	strcpy(d3.strs[3], "4");

	d4.cnt = 4;
	strcpy(d4.strs[0], "5");
	strcpy(d4.strs[1], "25");
	strcpy(d4.strs[2], "125");
	strcpy(d4.strs[3], "625");

	start(&thread1, 1, &d1);
	start(&thread2, 2, &d2);
	start(&thread3, 3, &d3);
	start(&thread4, 4, &d4);

	pthread_exit(NULL);
}