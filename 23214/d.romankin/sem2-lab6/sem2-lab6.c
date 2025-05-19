#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#define STR_NUM 100


void* threadBody(void* args) {
	char* string = args;
	int len = strlen(string);
	usleep(500*100*len);
	printf("\"%s\" \n", string);
	return NULL;

}

int main() {
	char** arr = malloc(STR_NUM * sizeof(char*));
	int str_count = 0;
	char buffer[BUFSIZ];
	while (1) {
		if (str_count > STR_NUM) {
			fprintf(stderr, "Number of strings limit exceeded\n");
			for (int i = 0; i < str_count; i++) {
					free(arr[i]);	
				}
			free(arr);
			exit(EXIT_FAILURE);
		}
		if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
			if (ferror(stdin)) {
				for (int i = 0; i < str_count; i++) {
					free(arr[i]);	
				}
				free(arr);
				exit(EXIT_FAILURE);
			}
			if (feof(stdin)) {
				break;
			}
			
		}
		arr[str_count] = malloc(strlen(buffer) * sizeof(char));
		buffer[strcspn(buffer,"\n")] = 0;
		strcpy(arr[str_count], buffer);
		str_count++;
	}
	
	pthread_t* threads = malloc(str_count * sizeof(pthread_t));

	int code;

	for (int i = 0; i < str_count; i++) {
		code = pthread_create(&threads[i], NULL, threadBody, arr[i]);
		if (code != 0) {
			fprintf(stderr, "creating thread error\n");
			for (int i = 0; i < str_count; i++) {
				free(arr[i]);	
			}
			free(arr);
			exit(EXIT_FAILURE);
		}
	}

	for (int i = 0; i < str_count; i++) {
		code = pthread_join(threads[i], NULL);
		if (code != 0) {
			fprintf(stderr, "thread join error\n");
			for (int i = 0; i < str_count; i++) {
				free(arr[i]);	
			}
			free(arr);
			exit(EXIT_FAILURE);
		}
	}

	for (int i = 0; i < str_count; i++) {
		free(arr[i]);	
	}
	free(arr);
	return 0;

}

