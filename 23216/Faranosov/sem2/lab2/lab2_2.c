#include "stdlib.h"
#include "pthread.h"
#include "stdio.h"
#include "string.h"

void* thread_funk() {
	for (int i = 0; i < 10; i++) {
		printf("%d\n", i);
	}
	return NULL;
}


int main() {
	pthread_t thread;

	int checkRes = 0;


	checkRes = pthread_create(&thread, NULL, thread_funk, NULL);
	if (checkRes != 0) {
		char buf[256];
		strerror_r(checkRes, buf, 256);
		fprintf(stderr, "create error: %s", buf);
		exit(1);
	}

	checkRes = pthread_join(thread, NULL);
	if (checkRes != 0) {
		char buf[256];
		strerror_r(checkRes, buf, 256);
		fprintf(stderr, "join error: %s", buf);
		exit(1);
	}

	thread_funk(NULL);
	exit(0);
}