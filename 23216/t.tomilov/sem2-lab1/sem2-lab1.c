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
    size_t strSize;
	pthread_t* thread = malloc(sizeof(pthread_t) * 11);
	for (int i = 0; i < 10; i++){
		if (getline(&str, &strSize, stdin) == -1){
			perror("ERROR: failed in getline!");
			exit(EXIT_FAILURE);
		}
		if (pthread_create(&(thread[i]), NULL, cat, str) != 0){
			perror("ERROR: failed in pthread_create!");
			exit(EXIT_FAILURE);
		}
	}
	for (int i = 0; i < 10; i++){
		pthread_join(thread[i], NULL);
	}
	free(thread);
	free(str);
	exit(EXIT_SUCCESS);
}