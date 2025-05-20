#include "stdlib.h"
#include "pthread.h"
#include "stdio.h"
#include "string.h"

void* thread_funk(void *args) {
	int isMain = (int)args;
	for (int i = 0; i < 10; i++) {
		if (isMain) printf("Main: ");
		printf("%d\n", i);
	}
	if (!isMain) pthread_exit(NULL);
	return NULL;
}

void handler(char str[], int num) {
	char buf[256];
	strerror_r(num, buf, 256);
	fprintf(stderr, "%s error: %s", str, buf);
	exit(1);
}

int main() {
	pthread_t thread;
	pthread_attr_t attr;

	int checkRes = 0;

	checkRes = pthread_attr_init(&attr);
	if (checkRes != 0) {
		handler("attr_init", checkRes);
	}
	
	int arg = 0;

	checkRes = pthread_create(&thread, &attr, thread_funk, (void*)arg);
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


	thread_funk((void*)1);

	exit(EXIT_SUCCESS);
}