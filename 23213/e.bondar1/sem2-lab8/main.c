#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define NUM_STEPS 200000000

typedef struct {
	int thread_index;
	int num_threads;
	double partial_sum;
} thread_data_t;

void *calculate_pi_partial(void *arg) {
	thread_data_t *data = (thread_data_t *)arg;
	int tid = data->thread_index;
	int n_threads = data->num_threads;

	int chunk_size = NUM_STEPS / n_threads;
	int start = tid * chunk_size;
	int end = (tid == n_threads - 1) ? NUM_STEPS : (tid + 1) * chunk_size;

	double local_sum = 0.0;
	for (int i = start; i < end; i++) {
		local_sum += 1.0/(i*4.0 + 1.0);
		local_sum -= 1.0/(i*4.0 + 3.0);
	}

	printf("Thread %d finished: partial sum = %.15g\n", tid, local_sum);
	data->partial_sum = local_sum;
	pthread_exit(NULL);
}

int main(int argc, char** argv) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <num_threads>\n", argv[0]);
		return EXIT_FAILURE;
	}

	int num_threads = atoi(argv[1]);
	if (num_threads <= 0) {
		fprintf(stderr, "Error: Number of threads must be positive\n");
		return EXIT_FAILURE;
	}

	pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
	thread_data_t *thread_data = malloc(num_threads * sizeof(thread_data_t));

	if (!threads || !thread_data) {
		perror("Failed to allocate memory");
		return EXIT_FAILURE;
	}

	for (int i = 0; i < num_threads; i++) {
		thread_data[i].thread_index = i;
		thread_data[i].num_threads = num_threads;

		int ret = pthread_create(&threads[i], NULL, calculate_pi_partial, &thread_data[i]);
		if (ret != 0) {
			fprintf(stderr, "Failed to create thread: %s\n", strerror(ret));
			exit(EXIT_FAILURE);
		}
	}

	double pi = 0.0;

	for (int i = 0; i < num_threads; i++) {
		int ret = pthread_join(threads[i], NULL);
		if (ret != 0) {
			fprintf(stderr, "Failed to join: %s\n", strerror(ret));
			exit(EXIT_FAILURE);
		}

		pi += thread_data[i].partial_sum;
	}

	pi = pi * 4.0;
	printf("pi done - %.16f \n", pi);

	free(threads);
	free(thread_data);

	return EXIT_SUCCESS;
}
