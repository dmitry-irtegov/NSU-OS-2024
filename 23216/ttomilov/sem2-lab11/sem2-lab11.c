#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

pthread_mutex_t mutex;
pthread_mutexattr_t mutexattr;

void err_handler(char* msg, int errID){
	fprintf(stderr, "%s %s\n", msg, strerror(errID));
}

void* threadFunc(){
	for (int i = 0; i < 10; i++){
		pthread_mutex_lock(&mutex);
		printf("Child: %d\n", i + 1);
		pthread_mutex_unlock(&mutex);
		usleep(1000);
	}
	pthread_exit(NULL);
}

int main(){
	int errID = 0;
	pthread_t thread;
	pthread_attr_t attr;

	if ((errID = pthread_mutexattr_init(&mutexattr)) != 0){
		err_handler("ERROR: failed in pthread_mutexattr_init. ", errID);
		exit(EXIT_FAILURE);
	}

	if ((errID = pthread_mutex_init(&mutex, &mutexattr)) != 0){
		err_handler("ERROR: failed in pthread_mutex_init. ", errID);
		exit(EXIT_FAILURE);
	}

	if ((errID = pthread_attr_init(&attr)) != 0){
		err_handler("ERROR: failed in pthread_attr_init. ", errID);
		exit(EXIT_FAILURE);
	}

	if ((errID = pthread_create(&thread, &attr, threadFunc, NULL)) != 0){
		err_handler("ERROR: failed in pthread_create. ", errID);
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < 10; i++){
		pthread_mutex_lock(&mutex);
		printf("Parent: %d\n", i + 1);
		pthread_mutex_unlock(&mutex);
		usleep(1000);
	}

	if ((errID = pthread_attr_destroy(&attr)) != 0){
		err_handler("ERROR: failed in pthread_attr_destroy. ", errID);
		exit(EXIT_FAILURE);
	}

	if ((errID = pthread_mutexattr_destroy(&mutexattr)) != 0){
		err_handler("ERROR: failed in pthread_mutexattr_destroy. ", errID);
		exit(EXIT_FAILURE);
	}

	pthread_exit(NULL);
}