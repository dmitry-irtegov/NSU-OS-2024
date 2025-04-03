#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define ITERATIONS 2000000000 

typedef struct {
    int start;
    int end;
    double partial_sum;
} thread_data_t;

void *calculate_partial_sum(void *arg) {
    thread_data_t *data = (thread_data_t *)arg;
    double sum = 0.0;

    for (int i = data->start; i < data->end; i++) {
        double term = (i % 2 == 0 ? 1.0 : -1.0) / (2 * i + 1);
        sum += term;
    }

    data->partial_sum = sum;

    pthread_exit((void *)data);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <number_of_threads>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int num_threads = atoi(argv[1]);
    if (num_threads <= 0) {
        printf("Number of threads must be positive.\n");
        return EXIT_FAILURE;
    }

    pthread_t threads[num_threads];
    thread_data_t *thread_data[num_threads];

    int iterations_per_thread = ITERATIONS / num_threads;
    int remaining_iterations = ITERATIONS % num_threads;

    time_t current_time;
    time(&current_time);

    for (int i = 0; i < num_threads; i++) {
        thread_data[i] = (thread_data_t *)malloc(sizeof(thread_data_t));
        if (thread_data[i] == NULL) {
            printf("Error allocating memory for thread data.\n");
            return EXIT_FAILURE;
        }

        thread_data[i]->start = i * iterations_per_thread;
        thread_data[i]->end = (i + 1) * iterations_per_thread;

        if (i == num_threads - 1) {
            thread_data[i]->end += remaining_iterations;
        }

        if (pthread_create(&threads[i], NULL, calculate_partial_sum, (void *)thread_data[i]) != 0) {
            printf("Error creating thread %d.\n", i);
            return EXIT_FAILURE;
        }
    }

    double pi = 0.0;
    for (int i = 0; i < num_threads; i++) {
        thread_data_t *result;
        if (pthread_join(threads[i], (void **)&result) != 0) {
            printf("Error joining thread %d.\n", i);
            return EXIT_FAILURE;
        }

        pi += result->partial_sum;
        free(result);
    }

    pi *= 4.0;

    time_t afterExecution_time;
    time(&afterExecution_time);
    int execution_time = difftime(afterExecution_time, current_time);

    printf("Approximated value of Pi: %.15f\n", pi);
    printf("Execution time with %d threads = %d seconds", num_threads, execution_time);

    return EXIT_SUCCESS;
}
