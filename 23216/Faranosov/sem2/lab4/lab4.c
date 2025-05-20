#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

void* func(void* param) {
	char* str = param;
	printf("%s\n", str);
	return NULL;
}


int main() {
	pthread_t thread;
	int res;

	res = pthread_create(&thread, NULL, func, "abc");
	if (res != 0) {
		perror("create error");
		exit(EXIT_FAILURE);
	}

	sleep(2);
	res = pthread_cancel(thread);
	if (res != 0) {
		perror("cancel error");
		exit(EXIT_FAILURE);
	}


	exit(EXIT_SUCCESS);
}