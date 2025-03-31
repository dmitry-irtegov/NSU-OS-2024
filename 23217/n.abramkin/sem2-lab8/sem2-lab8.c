#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_STEPS 200000000

typedef struct {
    int thread_id;
    int num_threads;
    double partial_sum;
} ThreadData;

void* calculate_pi(void* arg) {
    ThreadData* data = (ThreadData*) arg;
    int thread_id = data->thread_id;
    int num_threads = data->num_threads;

    double sum = 0.0;
    for (int i = thread_id; i < NUM_STEPS; i += num_threads) {
        double term = 1.0 / (i * 4.0 + 1.0) - 1.0 / (i * 4.0 + 3.0);
        sum += term;
    }

    data->partial_sum = sum;
    pthread_exit(NULL);
}

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <number_of_threads>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int num_threads = atoi(argv[1]);
    if (num_threads <= 0) {
        fprintf(stderr, "Error: The number of threads should be > 0\n");
        return EXIT_FAILURE;
    }

    pthread_t threads[num_threads];
    ThreadData thread_data[num_threads];

    for (int i = 0; i < num_threads; i++) {
        thread_data[i].thread_id = i;
        thread_data[i].num_threads = num_threads;
        thread_data[i].partial_sum = 0.0;
        pthread_create(&threads[i], NULL, calculate_pi, &thread_data[i]);
    }

    double pi = 0.0;
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
        pi += thread_data[i].partial_sum;
    }

    pi *= 4.0;
    printf("pi done - %.15g \n", pi);

    return EXIT_SUCCESS;
}
