#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>


void cancelHandle(void* args) {
	char buf[23] = "Finishing child thread\n";
	write(1, buf, 23);

}

void* threadBody(void* args) {
	pthread_cleanup_push(cancelHandle, NULL);
	char buf[6] = "Child\n";
	while(1) {
		write(1, buf, 6);
	}
	pthread_cleanup_pop(0);
	return NULL;

}



int main() {
	pthread_t thread;
	int code = pthread_create(&thread, NULL, threadBody, NULL);
	if (code != 0) {
                fprintf(stderr, "Creating thread error\n");
                exit(EXIT_FAILURE);
	}

	sleep(2);
	if (pthread_cancel(thread) != 0) {
		fprintf(stderr, "thread cancel error\n");
		exit(EXIT_FAILURE);
	}

	if (pthread_join(thread, NULL) != 0) {
		fprintf(stderr, "thread join error\n");
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);

}

