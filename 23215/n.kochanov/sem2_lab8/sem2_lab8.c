#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

#define TOTAL_ITERATIONS 200000000

typedef struct thread_data {
    long long thread_id;
    double sum_partial;
} thread_data;

int thread_count;

void *compute_pi(void *arg) {
    double local_pi = 0.0;
    long long i;

    for (i = ((thread_data*)arg)->thread_id; i < TOTAL_ITERATIONS; i += thread_count) {
        local_pi += 1.0 / (i * 4.0 + 1.0);
        local_pi -= 1.0 / (i * 4.0 + 3.0);
    }
    ((thread_data*)arg)->sum_partial = local_pi;
    return arg;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <num_threads>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    thread_count = atoi(argv[1]);

    if (thread_count < 1) {
        fprintf(stderr, "Error: Number of threads must be at least 1\n");
        exit(EXIT_FAILURE);
    }

    double pi_value = 0;
    pthread_t *threads = malloc(thread_count * sizeof(pthread_t));
    thread_data *args = malloc(thread_count * sizeof(thread_data));

    if (!threads || !args) {
        perror("Memory allocation failed");
        free(threads);
        free(args);
        exit(EXIT_FAILURE);
    }
    
    int i = 0;
    for (i = 0; i < thread_count; i++) {
        args[i].thread_id = i;
        pthread_create(&threads[i], NULL, compute_pi, (void*)&args[i]);
    }

    for (i = 0; i < thread_count; i++) {
        thread_data* result;
        pthread_join(threads[i], (void **)&result);
        pi_value += result->sum_partial;
    }

    pi_value *= 4.0;
    printf("Calculated PI: %.15g \n", pi_value); 

    free(threads);
    free(args);

    return (EXIT_SUCCESS);
} 