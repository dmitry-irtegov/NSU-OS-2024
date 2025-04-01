#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

pthread_mutex_t mutex1;
pthread_mutex_t mutex2;
pthread_mutex_t mutex3;

void err_handler(char* msg, int errID){
	fprintf(stderr, "%s %s\n", msg, strerror(errID));
}

void* threadFunc(){
	int errID = 0;
	if ((errID = pthread_mutex_lock(&mutex3))) {
		err_handler("pthread_mutex_lock", errID);
		exit(EXIT_FAILURE);
	}
	for (int i = 0; i < 10; i++){
		if ((errID = pthread_mutex_lock(&mutex1))) {
			err_handler("pthread_mutex_lock", errID);
			exit(EXIT_FAILURE);
		}

		printf("Child: %d\n", i + 1);

		if ((errID = pthread_mutex_unlock(&mutex3))) {
			err_handler("ERROR: failed in pthread_mutex_lock. ", errID);
			exit(EXIT_FAILURE);
		}

		if ((errID = pthread_mutex_lock(&mutex2))) {
			err_handler("pthread_mutex_lock", errID);
			exit(EXIT_FAILURE);
		}

		if ((errID = pthread_mutex_unlock(&mutex1))) {
			err_handler("ERROR: failed in pthread_mutex_lock. ", errID);
			exit(EXIT_FAILURE);
		}

		if ((errID = pthread_mutex_lock(&mutex3))) {
			err_handler("pthread_mutex_lock", errID);
			exit(EXIT_FAILURE);
		}

		if ((errID = pthread_mutex_unlock(&mutex2))) {
			err_handler("ERROR: failed in pthread_mutex_lock. ", errID);
			exit(EXIT_FAILURE);
		}
	}
	pthread_exit(NULL);
}

int main(){
	int errID = 0;
	pthread_t thread;
	pthread_attr_t attr;
	pthread_mutexattr_t mutexattr;

	if ((errID = pthread_mutexattr_init(&mutexattr)) != 0){
		err_handler("ERROR: failed in pthread_mutexattr_init. ", errID);
		exit(EXIT_FAILURE);
	}

	if ((errID = pthread_mutex_init(&mutex1, &mutexattr)) != 0){
		err_handler("ERROR: failed in pthread_mutex_init. ", errID);
		exit(EXIT_FAILURE);
	}

	if ((errID = pthread_mutex_init(&mutex2, &mutexattr)) != 0){
		err_handler("ERROR: failed in pthread_mutex_init. ", errID);
		exit(EXIT_FAILURE);
	}

	if ((errID = pthread_mutex_init(&mutex3, &mutexattr)) != 0){
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

	if ((errID = pthread_mutex_lock(&mutex1)) != 0) {
		err_handler("ERROR: failed in pthread_mutex_lock. ", errID);
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < 10; i++){
		printf("Parent: %d\n", i + 1);
		if ((errID = pthread_mutex_lock(&mutex2))) {
			err_handler("ERROR: failed in pthread_mutex_lock. ", errID);
			exit(EXIT_FAILURE);
		}

		if ((errID = pthread_mutex_unlock(&mutex1))) {
			err_handler("ERROR: failed in pthread_mutex_lock. ", errID);
			exit(EXIT_FAILURE);
		}

		if ((errID = pthread_mutex_lock(&mutex3))) {
			err_handler("pthread_mutex_lock", errID);
			exit(EXIT_FAILURE);
		}

		if ((errID = pthread_mutex_unlock(&mutex2))) {
			err_handler("ERROR: failed in pthread_mutex_lock. ", errID);
			exit(EXIT_FAILURE);
		}

		if ((errID = pthread_mutex_lock(&mutex1))) {
			err_handler("pthread_mutex_lock", errID);
			exit(EXIT_FAILURE);
		}

		if ((errID = pthread_mutex_unlock(&mutex3))) {
			err_handler("ERROR: failed in pthread_mutex_lock. ", errID);
			exit(EXIT_FAILURE);
		}
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