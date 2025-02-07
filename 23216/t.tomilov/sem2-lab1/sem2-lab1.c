#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void* threadFunc(){
	printf("Child:\n");
	for (int i = 0; i < 10; i++){
		printf("	%d\n", i + 1);
	}
	pthread_exit(EXIT_SUCCESS);
}

int main(){
	int errID = 0;
	pthread_t thread;
	pthread_attr_t attr;

	if ((errID = pthread_attr_init(&attr)) != 0){
		fprintf(stderr, "ERROR: failed in pthread_attr_init. Program ended with code %d\n", errID);
		exit(EXIT_FAILURE);
	}

	if ((errID = pthread_create(&thread, &attr, threadFunc, NULL)) != 0){
		fprintf(stderr, "ERROR: failed in pthread_create. Program ended with code %d\n", errID);
		exit(EXIT_FAILURE);
	}

	printf("Parent\n");

	pthread_exit(EXIT_SUCCESS);
}