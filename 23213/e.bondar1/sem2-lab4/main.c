#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

void* child_thread_function(void* arg) {
	while (1) {
		printf("Child thread is working...\n");
		sleep(1);
	}
	return NULL;
}

int main() {
	pthread_t child_thread;
	int ret;

	ret = pthread_create(&child_thread, NULL, child_thread_function, NULL);
	if (ret != 0) {
		fprintf(stderr, "Failed to create thread: %s\n", strerror(ret));
		exit(EXIT_FAILURE);
	}

	sleep(2);

	ret = pthread_cancel(child_thread);
	if (ret != 0) {
		fprintf(stderr, "Failed to cancel thread: %s\n", strerror(ret));
		exit(EXIT_FAILURE);
	}

	pthread_join(child_thread, NULL);

	printf("Parent finished\n");
	exit(EXIT_SUCCESS);
}
