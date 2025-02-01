#include "stdlib.h"
#include "pthread.h"
#include "stdio.h"

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
		perror("pthread_create error");
		exit(1);
	}

	thread_funk();
	pthread_exit(NULL);
}