#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#define num_steps 200000000

typedef struct threadStruct_t
{
	int start;
	double sum;

} threadStruct;


int threadNum;

void* threadBody(void* args) {
	threadStruct* data = (threadStruct*) args;
	double pi = 0;
	for (int i = data->start; i < num_steps ; i += threadNum) {
		pi += 1.0/(i*4.0 + 1.0);
		pi -= 1.0/(i*4.0 + 3.0);
	}
	data->sum = pi;
	pthread_exit((data));
}





int main(int argc, char** argv) {
	if (argc != 2) {
		fprintf(stderr, "Usage: ./sem2-lab8 <number_of_threads>\n");
		exit(EXIT_FAILURE);
	}
	threadNum = atoi(argv[1]);
	threadStruct** data = (threadStruct**)malloc(sizeof(threadStruct*) * threadNum);
	if (data == NULL) {
		perror("threadStruct malloc error");
		exit(EXIT_FAILURE);
	}
	pthread_t* threads = (pthread_t*)malloc(sizeof(pthread_t) * threadNum);
	if (threads == NULL) {
		perror("threads array error");
		exit(EXIT_FAILURE);
	}
	int code;
	for (int i = 0; i < threadNum; i++) {
		data[i] = (threadStruct*)malloc(sizeof(threadStruct));
		if (data[i] == NULL) {
			perror("data[i] malloc error");
			exit(EXIT_FAILURE);
		}
		data[i]->start = i;
		code = pthread_create(&threads[i], NULL, threadBody, data[i]);
		if (code != 0) {
			fprintf(stderr, "error creating thread\n");
			exit(EXIT_FAILURE);
		}
	} 
	double res = 0;  
	for (int i = 0; i < threadNum; i++) {
		code = pthread_join(threads[i], (void*)&data[i]);
		if (code != 0) {
			fprintf(stderr, "pthread_join error\n");
			exit(EXIT_FAILURE);
		}
		res += data[i]->sum;
	} 
	res = res * 4.0;
	printf("pi done - %.15g \n", res);
	for(int i = 0; i < threadNum; i++) {
		if (data[i] != NULL) {
			free(data[i]);
		}
	}
	free(data);
	free(threads);
	exit(EXIT_SUCCESS);
}

