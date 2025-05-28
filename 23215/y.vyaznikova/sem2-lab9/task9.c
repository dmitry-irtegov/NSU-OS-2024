#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>

#define NUM_STEPS 200000000
#define CHECK_SIGNAL_EVERY 1000000

typedef struct {
    long long thread_index;
    double partial_sum;
    long long iterations_done;
} thread_data;

int num_threads;
int should_exit = 0;
int was_interrupted = 0;
pthread_mutex_t exit_mutex;

void sigint_handler(int signum) {
    (void)signum;
    pthread_mutex_lock(&exit_mutex);
    should_exit = 1;
    was_interrupted = 1;
    pthread_mutex_unlock(&exit_mutex);
}

void *calculate_partial_sum(void *thread_data_ptr) {
    double local_pi = 0.0;
    long long iterations = 0;
    long long start_index = ((thread_data *)thread_data_ptr)->thread_index;

    for (long long i = start_index; i < NUM_STEPS; i += num_threads) {
        local_pi += 1.0 / (i * 4.0 + 1.0);
        local_pi -= 1.0 / (i * 4.0 + 3.0);
        iterations++;

        if (iterations % CHECK_SIGNAL_EVERY == 0) {
            int should_stop;
            pthread_mutex_lock(&exit_mutex);
            should_stop = should_exit;
            pthread_mutex_unlock(&exit_mutex);
            if (should_stop) {
                ((thread_data *)thread_data_ptr)->partial_sum = local_pi;
                ((thread_data *)thread_data_ptr)->iterations_done = iterations;
                return thread_data_ptr;
            }
        }
    }

    ((thread_data *)thread_data_ptr)->partial_sum = local_pi;
    ((thread_data *)thread_data_ptr)->iterations_done = iterations;
    return thread_data_ptr;
}

int main(int argc, char **argv) {
    double pi = 0.0;
    pthread_t *thread_handles;
    thread_data *thread_data_array;
    long long total_iterations = 0;

    if (pthread_mutex_init(&exit_mutex, NULL) != 0) {
        fprintf(stderr, "Mutex initialization failed\n");
        exit(EXIT_FAILURE);
    }

    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        fprintf(stderr, "Cannot set signal handler\n");
        pthread_mutex_destroy(&exit_mutex);
        exit(EXIT_FAILURE);
    }

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
        thread_data_array[i].iterations_done = 0;
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
        total_iterations += result->iterations_done;
    }

    pi *= 4.0;
    if (was_interrupted) {
        printf("\nCalculated pi = %.16f (using %lld iterations)\n", pi, total_iterations);
    } else {
        printf("Calculated pi = %.16f (using %lld iterations)\n", pi, total_iterations);
    }

    free(thread_data_array);
    free(thread_handles);

    pthread_mutex_destroy(&exit_mutex);
    return EXIT_SUCCESS;
}