#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define NUM_STEPS 2000000000
int n_threads;

typedef struct {
    int thread_index;
    double partial_sum;
} thread_data_t;

void *calculate_pi_partial(void *arg) {
    thread_data_t *data = (thread_data_t *)arg;
    int tid = data->thread_index;

    double local_sum = 0.0;

    for (long long i = tid; i < NUM_STEPS; i += n_threads) {
        local_sum += 1.0 / (i * 4.0 + 1.0);
        local_sum -= 1.0 / (i * 4.0 + 3.0);
    }

    data->partial_sum = local_sum;
    pthread_exit(NULL);
}

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <num_threads>\n", argv[0]);
        return EXIT_FAILURE;
    }

    n_threads = atoi(argv[1]);
    if (n_threads <= 0) {
        fprintf(stderr, "Number of threads must be positive\n");
        return EXIT_FAILURE;
    }

    pthread_t *threads = malloc(n_threads * sizeof(pthread_t));
    thread_data_t *thread_data = malloc(n_threads * sizeof(thread_data_t));


    for (int i = 0; i < n_threads; i++) {
        thread_data[i].thread_index = i;
        thread_data[i].partial_sum = 0.0;

        int ret = pthread_create(&threads[i], NULL, calculate_pi_partial, &thread_data[i]);
        if (ret != 0) {
            fprintf(stderr, "Failed to create thread: %s\n", strerror(ret));
            exit(EXIT_FAILURE);
        }
    }

    double pi = 0.0;
    for (int i = 0; i < n_threads; i++) {
        int ret = pthread_join(threads[i], NULL);
        if (ret != 0) {
            fprintf(stderr, "Failed to join thread: %s\n", strerror(ret));
            exit(EXIT_FAILURE);
        }
        pi += thread_data[i].partial_sum;
    }

    pi *= 4.0;
    printf("Computed Pi = %.16f\n", pi);

    return EXIT_SUCCESS;
}
