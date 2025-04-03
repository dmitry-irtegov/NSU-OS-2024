#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <stdatomic.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int turn = 0;
void* thread_body(void* param) {
	for (int i = 0; i < 10; i++) {
		pthread_mutex_lock(&mutex);
		while (atomic_load(&turn) != 1) {
			pthread_cond_wait(&cond, &mutex);
		}
		printf("left\n");
		atomic_store(&turn, 0);
		pthread_cond_signal(&cond);
		pthread_mutex_unlock(&mutex);
	}
	return NULL;
}
int main(int argc, char* argv[]) {
	pthread_t thread;
	int code;
	pthread_mutex_init(&mutex, NULL);

	if ((code = pthread_create(&thread, NULL, thread_body, NULL))) {
		char buf[256];
		strerror_r(code, buf, sizeof(buf));
		fprintf(stderr, "%s: creating thread: %s\n", argv[0], buf);
		exit(1);
	}
	else {
		for (int i = 0; i < 10; i++) {
			pthread_mutex_lock(&mutex);
				while (atomic_load(&turn) != 0) {
					pthread_cond_wait(&cond, &mutex);
				}
				printf("right\n");
				atomic_store(&turn, 1);
				pthread_cond_signal(&cond);
			pthread_mutex_unlock(&mutex);
		}
	}
	pthread_join(thread, NULL);
	pthread_mutex_destroy(&mutex);
	pthread_exit(NULL);
}