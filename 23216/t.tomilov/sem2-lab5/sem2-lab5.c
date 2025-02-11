#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

void err_hendler(char* msg, int errID){
	fprintf(stderr, "%s %s\n", msg, strerror(errID));
}

void cancel(){
	printf("cancel\n");
}

void* pthreadFunc(void* data){
	pthread_cleanup_push(cancel, "cancel");
	char* str = *((char**) data);
	while(1){
		printf("%s", str);
	}
	pthread_cleanup_pop(1);
	pthread_exit(0);
}

int main(){
	pthread_t thread;
	pthread_attr_t attr;
	int errID = 0;
	
	char* str = NULL;
	size_t len = 0;

	if ((errID = pthread_attr_init(&attr)) != 0){
		err_hendler("ERROR: failed to init attr. Program ended with code", errID);
		exit(EXIT_FAILURE);
	}

	if ((errID = getline(&str, &len, stdin)) < 0){
		err_hendler("ERROR: failed in getline. Program ended with code", errID);
		exit(EXIT_FAILURE);
	}

	if ((errID = pthread_create(&thread, &attr, pthreadFunc, (void*) &str)) != 0) {
		err_hendler("ERROR: failed to create thread. Program ended with code", errID);
		exit(EXIT_FAILURE);
	}

	sleep(2);

	if ((errID = pthread_cancel(thread)) != 0) {
		err_hendler("ERROR: failed to cancel thread. Program ended with code", errID);
		exit(EXIT_FAILURE);
	}

	if ((errID = pthread_attr_destroy(&attr)) != 0) {
        err_hendler("ERROR: failed to destroy the attr. Program ended with code", errID);
        exit(EXIT_FAILURE);
    }
	pthread_exit(0);
}
	