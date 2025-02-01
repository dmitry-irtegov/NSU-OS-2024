#include "stdio.h"
#include "stdlib.h"
#include "pthread.h"
#include "string.h"

typedef struct data {
	char strs[4][20];
	int cnt;
} data;

data d1;

void* func(void *param) {
	
	data* d = param;
	for (int i = 0; i < d->cnt; i++) {
		printf("%s\n", d->strs[i]);
	}

	return NULL;
}


int main() {
	pthread_t thread1;
	int res;

	d1.cnt = 4;
	strcpy(d1.strs[0], "a");
	strcpy(d1.strs[1], "b");
	strcpy(d1.strs[2], "c");
	strcpy(d1.strs[3], "d");
	


	res = pthread_create(&thread1, NULL, func, &d1);
	if (res != 0) {
		perror("create 1 error");
		exit(1);
	}

	pthread_exit(NULL);
}