#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>
#include <assert.h>
sem_t sem2;
sem_t sem1;
void* threadBody(void* args) {
	
	for (int i = 0; i < 10; i++) {
		assert(sem_wait(&sem2) == 0);
		printf("Child %d\n", i);
		assert(sem_post(&sem1) == 0);
	}

	return NULL;

}

void mainThread() {
	for (int i = 0; i < 10; i++) {
		assert(sem_wait(&sem1) == 0);
		printf("Parent %d\n", i);
		assert(sem_post(&sem2) == 0);		
	}


}

int main() {
	pthread_t thread;
	if (sem_init(&sem2, 0, 0) != 0) {
		perror("semaphore 2 init error");
		exit(EXIT_FAILURE);
	}
	if (sem_init(&sem1, 0, 1) != 0) {
		perror("semaphore 1 init error");
		exit(EXIT_FAILURE);
	}
	int code = pthread_create(&thread, NULL, threadBody, NULL);
	if (code != 0) {
		fprintf(stderr, "Creating thread error\n");
		exit(EXIT_FAILURE);
	}
	mainThread();
	code = pthread_join(thread, NULL);
	if (code != 0) {
		fprintf(stderr, "Joining thread error\n");
		exit(EXIT_FAILURE);
	}
	if (sem_destroy(&sem1) != 0) {
		perror("semaphore 1 destroy error");
		exit(EXIT_FAILURE);
	}
	if (sem_destroy(&sem2) != 0) {
		perror("semaphore 2 destroy error");
		exit(EXIT_FAILURE);
	}
	exit(EXIT_SUCCESS);
}
