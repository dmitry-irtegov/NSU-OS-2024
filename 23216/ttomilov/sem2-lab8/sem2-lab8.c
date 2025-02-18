#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

void* pi_serial(void* data){
	int* sten = (int*) data;
	double* result = malloc(sizeof(double));
	if (result == NULL){
		pthread_exit(NULL);
	}
	for (int i = sten[0]; i < sten[1]; i++){
		*result += 1.0/(i*4.0 + 1.0);
        *result -= 1.0/(i*4.0 + 3.0);
	}

	pthread_exit((void*) result);
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
	void* inter_res;
	double result = 0;

	int** sten = malloc(sizeof(int*) * num_threads);
	if (sten == NULL){
		perror("ERRRO: failed to allocate memory!");
		exit(EXIT_FAILURE);
	}

	pthread_t* threads = malloc(sizeof(pthread_t) * num_threads);
	if (threads == NULL){
		perror("ERRRO: failed to allocate memory!");
		free(sten);
		exit(EXIT_FAILURE);
	}

	pthread_attr_t* attrs = malloc(sizeof(pthread_attr_t) * num_threads);
	if (attrs == NULL){
		perror("ERRRO: failed to allocate memory.");
		free(threads);
		free(sten);
		exit(EXIT_FAILURE);
	}

	sten[0] = malloc(sizeof(int) * 2);
	if (sten[0] == NULL){
		perror("ERRRO: failed to allocate memory.");
		free(threads);
		free(sten);
		exit(EXIT_FAILURE);
	}
	sten[0][0] = 0;
	sten[0][1] = steps;

	for (int i = 1; i < num_threads; i++){
		sten[i] = malloc(sizeof(int) * 2);
		if (sten[i] == NULL){
			perror("ERRRO: failed to allocate memory.");
			free(threads);
			for (int j = 0; j < i; j++){
				free(sten[j]);
			}
			free(sten);
			exit(EXIT_FAILURE);
		}

		sten[i][0] = sten[i - 1][1];
		sten[i][1] = sten[i][0] + rest;
	}

	if (sten[num_threads - 1][1] > num_elem_row){
		sten[num_threads - 1][1] = num_elem_row;
	}

	for (int i = 0; i < num_threads; i++){
		//printf("%d %d\n", sten[0], sten[1]);
		if ((errID = pthread_create(&(threads[i]), &attrs[i], &pi_serial, (void*) sten[i])) != 0){
			fprintf(stderr, "ERROR: failed to create a thread. %s\n", strerror(errID));
			for (int j = 0; j < num_threads; j++){
				free(sten[i]);
			}
			free(sten);
			exit(EXIT_FAILURE);
		}
	}

	for (int i = 0; i < num_threads; i++){
		if ((errID = pthread_join(threads[i], &inter_res)) != 0){
			fprintf(stderr, "ERROR: failed to join thread. %s\n", strerror(errID));
			for (int j = 0; j < num_threads; j++){
				free(sten[i]);
			}
			free(sten);
			exit(EXIT_FAILURE);
		}
		
		result += *(double*) inter_res;
	}

	result *= 4.0;
	printf("Result: %.5le\n", result);
	exit(EXIT_SUCCESS);
}