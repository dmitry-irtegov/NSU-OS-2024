#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_STEPS 200000000L

typedef struct {
	long long start_index;
	double partial_sum;
} ThreadData;

int num_threads;

void *compute_pi_segment(void *arg) {
	ThreadData *data = (ThreadData *)arg;
	double sum = 0.0;

	for (long long i = data->start_index; i < NUM_STEPS; i += num_threads) {
		double term1 = 1.0 / (4.0 * i + 1.0);
		double term2 = 1.0 / (4.0 * i + 3.0);
		sum += term1 - term2;
	}

	data->partial_sum = sum;

	fprintf(stderr, "[Thread %lld] Partial sum: %.16f\n", data->start_index, sum);

	return data;
}

int main(int argc, char *argv[]) {
	if (argc != 2 || (num_threads = atoi(argv[1])) <= 0) {
		fprintf(stderr, "Usage: %s <num_threads>\n", argv[0]);
		return EXIT_FAILURE;
	}

	pthread_t *threads = malloc(sizeof(pthread_t) * num_threads);
	ThreadData *thread_data = malloc(sizeof(ThreadData) * num_threads);

	if (!threads || !thread_data) {
		fprintf(stderr, "Memory allocation failed\n");
		return EXIT_FAILURE;
	}

	for (int i = 0; i < num_threads; ++i) {
		thread_data[i].start_index = i;
		if (pthread_create(&threads[i], NULL, compute_pi_segment, &thread_data[i]) != 0) {
			perror("pthread_create");
			return EXIT_FAILURE;
		}
	}

	double pi = 0.0;
	for (int i = 0; i < num_threads; ++i) {
		ThreadData *result;
		if (pthread_join(threads[i], (void **)&result) != 0) {
			perror("pthread_join");
			return EXIT_FAILURE;
		}
		pi += result->partial_sum;
	}

	pi *= 4.0;
	printf("pi = %.14f (computed with %d threads)\n", pi, num_threads);

	free(threads);
	free(thread_data);
	return EXIT_SUCCESS;
}

