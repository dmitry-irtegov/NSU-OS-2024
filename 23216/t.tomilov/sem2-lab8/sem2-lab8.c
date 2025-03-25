#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

pthread_mutex_t mutex;
double* inter_res;

typedef struct Args{
	int start;
	int end;
	int thread_id;
	int done;
	int isCounted;
}Args;

void* pi_serial(void* data){
	Args* args = (Args*) data;
	double result = 0;
	for (int i = args->start; i < args->end; i++){
		result += 1.0/(i*4.0 + 1.0);
        result -= 1.0/(i*4.0 + 3.0);
	}
	inter_res[args->thread_id] = result;
	args->done = 1;
	pthread_exit(NULL);
}

int main(int argc, char** argv){
	if (argc < 2){
		fprintf(stderr, "ERROR: not enough arguments. Try %s <num_of_threads>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	int errID = 0;
	int num_threads = atoi(argv[1]);
	int num_elem_row = 200000000;
	int steps = num_elem_row/num_threads;
	int rest = num_elem_row%num_threads;
	double result = 0;
	int count = 0;
	pthread_attr_t attr;
	pthread_mutexattr_t mutexattr;

	Args* args = malloc(sizeof(Args) * num_threads);
	if (args == NULL){
		perror("ERROR: failed to allocate memory!");
		exit(EXIT_FAILURE);
	}

	pthread_t* threads = malloc(sizeof(pthread_t) * num_threads);
	if (threads == NULL){
		perror("ERRRO: failed to allocate memory!");
		free(args);
		exit(EXIT_FAILURE);
	}

	inter_res = malloc(sizeof(double) * num_threads);
	if (inter_res == NULL){
		perror("ERRRO: failed to allocate memory.");
		free(threads);
		free(args);
		exit(EXIT_FAILURE);
	}

	if (pthread_mutexattr_init(&mutexattr) != 0){
		perror("ERROR: failed to init mutexattr.");
		free(threads);
		free(args);
		free(inter_res);
		exit(EXIT_FAILURE);
	}

	if ((errID = pthread_mutex_init(&mutex, &mutexattr)) != 0){
		perror("ERROR: failed to init mutex.");
		free(threads);
		free(args);
		free(inter_res);
		exit(EXIT_FAILURE);
	}

	if ((errID = pthread_attr_init(&attr) != 0)){
		fprintf(stderr, "ERROR: failed to init attr. %s\n", strerror(errID));
		free(threads);
		free(args);
		free(inter_res);
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < num_threads; i++){
		args[i].start = args[i - 1].end;
		args[i].end = args[i].end + steps + rest;
		args[i].thread_id = i;
		args[i].done = 0;
		args[i].isCounted = 0;
	}

	if (args[num_threads - 1].end > num_elem_row){
		args[num_threads - 1].end = num_elem_row;
	}

	for (int i = 0; i < num_threads; i++){
		if ((errID = pthread_create(&(threads[i]), &attr, &pi_serial, (void*) &args[i])) != 0){
			fprintf(stderr, "ERROR: failed to create a pthread. %s\n", strerror(errID));
			free(threads);
			free(args);
			free(inter_res);
			exit(EXIT_FAILURE);
		}
	}

	while(count < num_threads){
		pthread_mutex_lock(&mutex);
		for (int i = 0; i < num_threads; i++){
			if (args[i].done == 1 && args[i].isCounted == 0){
				result += inter_res[args[i].thread_id];
				args[i].isCounted = 1;
				count++;
			}
		}
		pthread_mutex_unlock(&mutex);
	}

	if ((errID = pthread_mutex_destroy(&mutex)) != 0){
		fprintf(stderr, "ERROR: failed to destroy mutex. %s\n", strerror(errID));
		free(threads);
		free(args);
		free(inter_res);
		exit(EXIT_FAILURE);
	}

	if ((errID = pthread_mutexattr_destroy(&mutexattr)) != 0){
		fprintf(stderr, "ERROR: failed to destroy mutexattr. %s\n", strerror(errID));
		free(threads);
		free(args);
		free(inter_res);
		exit(EXIT_FAILURE);
	}

	if ((errID = pthread_attr_destroy(&attr)) != 0){
		fprintf(stderr, "ERROR: failed to destroy attr. %s\n", strerror(errID));
		free(threads);
		free(args);
		free(inter_res);
		exit(EXIT_FAILURE);
	}

	result *= 4.0;
	printf("Result: %.5le\n", result);
	free(threads);
	free(args);
	free(inter_res);
	exit(EXIT_SUCCESS);
}