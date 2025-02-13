#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
void* threadBody(void* args) {
	for (int i = 0; i < 10; i++) {
		printf("Child\n");
	}
	return NULL;

}

void mainThread() {
	for (int i = 0; i < 10; i++) {
		printf("Parent\n");
	}

}

int main() {
	pthread_t thread;
	int code = pthread_create(&thread, NULL, threadBody, NULL);
	if (code != 0) {
                fprintf(stderr, "Creating thread error\n");
                exit(EXIT_FAILURE);
	}
	if (pthread_join(thread, NULL) != 0) {
		fprintf(stderr, "pthread_join error\n");
		exit(EXIT_FAILURE);
	}
	mainThread();
	return 0;

}

