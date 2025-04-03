#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct {
    long long thread_index;
    double partial_sum;
} thread_data;

#define NUM_STEPS 200000000

int num_threads;

void *calculate_partial_sum(void *thread_data_ptr) {
    double local_pi = 0.0;
    long long start_index = ((thread_data *)thread_data_ptr)->thread_index;

    for (long long i = start_index; i < NUM_STEPS; i += num_threads) {
        local_pi += 1.0 / (i * 4.0 + 1.0);
        local_pi -= 1.0 / (i * 4.0 + 3.0);
    }

    fprintf(stderr, "Thread %lld finished, partial sum %.16f\n",
            start_index, local_pi);

    ((thread_data *)thread_data_ptr)->partial_sum = local_pi;
    return thread_data_ptr;
}

int main(int argc, char **argv) {
    double pi = 0.0;
    pthread_t *thread_handles;
    thread_data *thread_data_array;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <number_of_threads>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    num_threads = atol(argv[1]);
    if (num_threads < 1) {
        fprintf(stderr, "Number of threads must be >= 1\n");
        exit(EXIT_FAILURE);
    }

    thread_data_array = malloc(num_threads * sizeof(thread_data));
    thread_handles = malloc(num_threads * sizeof(pthread_t));
    if (thread_data_array == NULL || thread_handles == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < num_threads; i++) {
        thread_data_array[i].thread_index = i;
        if (pthread_create(&thread_handles[i], NULL, calculate_partial_sum, (void *)(&thread_data_array[i])) != 0) {
            fprintf(stderr, "Thread creation failed\n");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < num_threads; i++) {
        thread_data *result;
        if (pthread_join(thread_handles[i], (void **)&result) != 0) {
            fprintf(stderr, "Thread join failed\n");
            exit(EXIT_FAILURE);
        }
        pi += result->partial_sum;
    }

    pi *= 4.0;
    printf("Calculated pi = %.16f\n", pi);

    free(thread_data_array);
    free(thread_handles);

    return EXIT_SUCCESS;
}