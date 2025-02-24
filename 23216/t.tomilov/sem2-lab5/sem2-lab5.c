#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>

sem_t sem;

void err_hendler(char* msg, int errID){
	fprintf(stderr, "%s %s\n", msg, strerror(errID));
}

void cancel(){
	printf("\ncancel\n");
	sem_post(&sem);
}

void* pthreadFunc(void* data){
	char* str = *((char**) data);
	pthread_cleanup_push(cancel, NULL);
	while(1){
		write(1, str, strlen(str));
	}
	pthread_cleanup_pop(1);
}

int main(){
	pthread_t thread;
	pthread_attr_t attr;
	int errID = 0;
	
	char* str = NULL;
	size_t len = 0;

	if ((errID = pthread_attr_init(&attr)) != 0){
		err_hendler("ERROR: failed to init attr.", errID);
		exit(EXIT_FAILURE);
	}

	if ((errID = sem_init(&sem, 0, 0)) != 0){
		err_hendler("ERROR: failed to init sem.", errID);
	}

	if ((errID = getline(&str, &len, stdin)) < 0){
		err_hendler("ERROR: failed in getline.", errID);
		exit(EXIT_FAILURE);
	}

	if ((errID = pthread_create(&thread, &attr, pthreadFunc, (void*) &str)) != 0) {
		err_hendler("ERROR: failed to create thread.", errID);
		free(str);
		exit(EXIT_FAILURE);
	}

	sleep(2);

	if ((errID = pthread_cancel(thread)) != 0) {
		err_hendler("ERROR: failed to cancel thread.", errID);
		free(str);
		exit(EXIT_FAILURE);
	}

	if ((errID = pthread_attr_destroy(&attr)) != 0) {
        err_hendler("ERROR: failed to destroy the attr.", errID);
		free(str);
        exit(EXIT_FAILURE);
	}

	if ((errID = sem_wait(&sem)) != 0){
		err_hendler("ERROR: failed in sem_wait.", errID);
		exit(EXIT_FAILURE);
	}

	free(str);
	str = NULL;
	if ((errID = sem_destroy(&sem)) != 0){
		err_hendler("ERROR: failed to destroy semaphore.", errID);
		exit(EXIT_FAILURE);
	}
	
	exit(EXIT_SUCCESS);
}