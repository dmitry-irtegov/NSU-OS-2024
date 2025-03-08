#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

pthread_mutex_t mtx;

void err_handler(char* msg, int errID){
	fprintf(stderr, "%s %s\n", msg, strerror(errID));
}

void cancel(){
	printf("\ncancel\n");
	pthread_mutex_unlock(&mtx);
}

void* pthreadFunc(void* data){
	char* str = *((char**) data);
	pthread_mutex_lock(&mtx);
	pthread_cleanup_push(cancel, NULL);
	while(1){
		write(1, str, strlen(str));
	}
	pthread_cleanup_pop(1);
}

int main(){
	pthread_t thread;
	pthread_attr_t attr;
	pthread_mutexattr_t mtxattr;
	int errID = 0;

	char* str = NULL;
	size_t len = 0;

	if ((errID = pthread_attr_init(&attr)) != 0){
		err_handler("ERROR: failed to init attr.", errID);
		exit(EXIT_FAILURE);
	}

	if ((errID = pthread_mutexattr_init(&mtxattr)) != 0){
		err_handler("ERROR: failed to init mutexattr.", errID);
		exit(EXIT_FAILURE);
	}

	if ((errID = pthread_mutex_init(&mtx, &mtxattr)) != 0){
		err_handler("ERROR: failed to init mutex.", errID);
		exit(EXIT_FAILURE);
	}

	if ((errID = getline(&str, &len, stdin)) < 0){
		err_handler("ERROR: failed in getline.", errID);
		exit(EXIT_FAILURE);
	}

	if ((errID = pthread_create(&thread, &attr, pthreadFunc, (void*) &str)) != 0) {
		err_handler("ERROR: failed to create thread.", errID);
		free(str);
		exit(EXIT_FAILURE);
	}

	sleep(2);

	if ((errID = pthread_cancel(thread)) != 0) {
		err_handler("ERROR: failed to cancel thread.", errID);
		free(str);
		exit(EXIT_FAILURE);
	}

	errID = pthread_mutex_lock(&mtx);
	while (errID != 0){
		errID = pthread_mutex_lock(&mtx);
	}
	pthread_mutex_unlock(&mtx);

	if ((errID = pthread_attr_destroy(&attr)) != 0) {
        err_handler("ERROR: failed to destroy the attr.", errID);
		free(str);
        exit(EXIT_FAILURE);
	}

	if ((errID = pthread_mutexattr_destroy(&mtxattr)) != 0) {
        err_handler("ERROR: failed to destroy the mutexattr.", errID);
		free(str);
        exit(EXIT_FAILURE);
	}

	free(str);
	exit(EXIT_SUCCESS);
}