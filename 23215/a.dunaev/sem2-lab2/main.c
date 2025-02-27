#include <stdio.h>
#include <pthread.h>

void *child(void * param){
	for (int j = 0; j < 10; j++){
		printf("\tChild process %d\n", j);
	}
	return param;
}

int main() {
	pthread_t thread;
	int code = pthread_create(&thread, NULL, &child, NULL);
	if (code != 0){
		perror("Thread not initialized!");
	}
	int error = pthread_join(thread, NULL);
	if (error != 0){
		perror("Thread cannot be awaited");
	}
	for (int i = 0; i < 10; i++){
		printf("Parent process %d\n", i);
	
	}
	pthread_exit(0);
}
