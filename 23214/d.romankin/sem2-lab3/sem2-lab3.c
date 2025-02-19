#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void* threadBody(void* args) {
	char** arr = (char**) args;
	while(*arr != NULL) {
		printf("%s\n", *arr);
		arr++;
	}
	return NULL;
}

int main() {
	char* strings1[] = {"thread1_line0", "thread1_line1", "thread1_line2", NULL};
	char* strings2[] = {"thread2_line0", "thread2_line1", NULL};
	char* strings3[] = {"thread3_line0", "thread3_line1", "thread3_line2", "thread3_line3",NULL};
	char* strings4[] = {"thread4_line0", NULL};

	char** strings[] = {strings1, strings2, strings3, strings4};
	int code;
	pthread_t threads[4];
	for (int i = 0; i < 4; i++) {
		code = pthread_create(&threads[i], NULL, threadBody, (void*) strings[i]);
		if (code != 0) {
			fprintf(stderr, "creating thread error\n");
			exit(EXIT_FAILURE);
		}
	}

	for (int i = 0; i < 4; i++) {
		code = pthread_join(threads[i], NULL);
		if (code != 0) {
			fprintf(stderr, "thread join error\n");
			exit(EXIT_FAILURE);
		}
	}
	return 0;
}
