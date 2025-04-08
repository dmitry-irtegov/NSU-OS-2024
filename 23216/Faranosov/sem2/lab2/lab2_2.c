#include "stdlib.h"
#include "pthread.h"
#include "stdio.h"
#include "string.h"

void* thread_funk() {
	for (int i = 0; i < 10; i++) {
		printf("%d\n", i);
	}
	pthread_exit(NULL);
}

void handler(char str[], int num) {
	char buf[256];
	strerror_r(num, buf, 256);
	fprintf(stderr, "%s error: %s", str, buf);
	exit(EXIT_FAILURE);
}

int main() {
	pthread_t thread;
	pthread_attr_t attr;

	int checkRes = 0;

	checkRes = pthread_attr_init(&attr);
	if (checkRes != 0) {
		handler("attr_init", checkRes);
	}

	checkRes = pthread_create(&thread, &attr, thread_funk, NULL);
	if (checkRes != 0) {
		handler("create", checkRes);
	}


	checkRes = pthread_attr_destroy(&attr);
	if (checkRes != 0) {
		handler("destroy", checkRes);
	}

	checkRes = pthread_join(thread, NULL);
	if (checkRes != 0) {
		handler("join", checkRes);
	}

	thread_funk();
	exit(EXIT_SUCCESS);
}
