#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

void err_handler(char* msg, int errID){
	fprintf(stderr, "%s %s\n", msg, strerror(errID));
}

void* threadFunc(){
	printf("Child:\n");
	for (int i = 0; i < 10; i++){
		printf("%d\n", i + 1);
	}
	pthread_exit(0);
}

int main(){
	int errID = 0;
	pthread_t thread;
	pthread_attr_t attr;

	if ((errID = pthread_attr_init(&attr)) != 0){
		err_handler("ERROR: failed in pthread_attr_init. Program ended with code", errID);
		exit(EXIT_FAILURE);
	}

	if ((errID = pthread_create(&thread, &attr, threadFunc, NULL)) != 0){
		err_handler("ERROR: failed in pthread_create. Program ended with code", errID);
		exit(EXIT_FAILURE);
	}

	if ((errID = pthread_attr_destroy(&attr)) != 0){
		err_handler("ERROR: failed in pthread_attr_destroy. Program ended with code", errID);
		exit(EXIT_FAILURE);
	}

	printf("Parent\n");

	pthread_exit(0);
}