#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

void* cat(void* threadData){
	char* str = (char*) threadData;
	printf("%s", str);
	return 0;
}

int main(){
	char* str = NULL;
    size_t strSize = 0;
	int errID = 0;
	pthread_t* thread = malloc(sizeof(pthread_t) * 11);
	if (thread == NULL){
		perror("ERROR: failed to allocate memory!");
		exit(EXIT_FAILURE);
	}
	for (int i = 0; i < 10; i++){
		if (getline(&str, &strSize, stdin) == -1){
			fprintf(stderr, "ERROR: failed in getline");
			exit(EXIT_FAILURE);
		}
		if ((errID = pthread_create(&(thread[i]), NULL, cat, str)) != 0){
			char buf[256];
			strerror_r(errID, buf, 256);
			fprintf(stderr, "ERROR: %s", buf);
			exit(EXIT_FAILURE);
		}
	}
	for (int i = 0; i < 10; i++){
		if ((errID = pthread_join(thread[i], NULL)) != 0){
			char buf[256];
			strerror_r(errID, buf, 256);
			fprintf(stderr, "ERROR: %s", buf);
			exit(EXIT_FAILURE);
		}
	}
	free(thread);
	free(str);
	exit(EXIT_SUCCESS);
}