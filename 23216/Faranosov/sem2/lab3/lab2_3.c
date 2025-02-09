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

void handler(char str[], int num, int number) {
	char buf[256];
	strerror_r(num, buf, 256);
	fprintf(stderr, "%s %d error: %s", str, num, buf);
	exit(EXIT_FAILURE);
}

void start(pthread_t* thr, int numb, data* d) {
	int res = 0;
	pthread_attr_t attr;
	res = pthread_attr_init(&attr);
	if (res != 0) {
		handler("init", res, numb);
	}
	
	res = pthread_create(thr, &attr, func, d);
	if (res != 0) {
		handler("create", res, numb);
	}

	res = pthread_attr_destroy(&attr);
	if (res != 0) {
		handler("destroy", res, numb);
	}
}

char* strs1[] = { "a", "b", "c", "d" };
char* strs2[] = { "00", "01", "10", "11" };
char* strs3[] = { "1", "2", "3", "4" };
char* strs4[] = { "5", "25", "125", "625" };


void set() {
	d1.cnt = d2.cnt = d3.cnt = d4.cnt = 4;
	for (int i = 0; i < 4; i++) {
		strcpy(d1.strs[i], strs1[i]);
		strcpy(d2.strs[i], strs2[i]);
		strcpy(d3.strs[i], strs3[i]);
		strcpy(d4.strs[i], strs4[i]);
	}
}


int main() {
	pthread_t thread1, thread2, thread3, thread4;

	set();

	start(&thread1, 1, &d1);
	start(&thread2, 2, &d2);
	start(&thread3, 3, &d3);
	start(&thread4, 4, &d4);

	pthread_exit(NULL);
}