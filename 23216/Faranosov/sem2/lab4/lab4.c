#include "stdlib.h"
#include "pthread.h"
#include "stdio.h"
#include "string.h"
#include "unistd.h"


void* thread_funk() {
	
	int res = 0;
	for (long long  i = 0;; i++) {
		res = write(1, "thread\n", 8);
		if (res < 0) {
			perror("write error");
			exit(EXIT_FAILURE);
		}
	}
	
	pthread_exit(NULL);
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

	checkRes = pthread_create(&thread, &attr, thread_funk, arg);
	if (checkRes != 0) {
		handler("create", checkRes);
	}


	checkRes = pthread_attr_destroy(&attr);
	if (checkRes != 0) {
		handler("destroy", checkRes);
	}

	
	checkRes = sleep(2);
	if (checkRes != 0) {
		fprintf(stderr, "sleep error: time elapsed == %u", checkRes);
		exit(EXIT_FAILURE);
	}

	checkRes = pthread_cancel(thread);
	if (checkRes != 0) {
		handler("cancel", checkRes);
	}


	
	pthread_exit(NULL);
}