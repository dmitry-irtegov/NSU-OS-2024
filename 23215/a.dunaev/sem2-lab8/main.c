#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define ITERATIONS 1000000000

typedef struct {
    int start;
    int end;
    double partial_sum;
} ThreadData;

void *calculate_pi(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    double sum = 0.0;
    for (int i = data->start; i < data->end; i++) {
        double term = (i % 2 == 0 ? 1.0 : -1.0) / (2 * i + 1);
        sum += term;
    }
    data->partial_sum = sum;
    pthread_exit((void *)&data->partial_sum);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <num_threads>\n", argv[0]);
        return 1;
    }

    int num_threads = atoi(argv[1]);
    pthread_t threads[num_threads];
    ThreadData thread_data[num_threads];
    double pi = 0.0;

    int chunk = ITERATIONS / num_threads;

    for (int i = 0; i < num_threads; i++) {
        thread_data[i].start = i * chunk;
        thread_data[i].end = (i == num_threads - 1) ? ITERATIONS : (i + 1) * chunk;
        pthread_create(&threads[i], NULL, calculate_pi, &thread_data[i]);
    }

    for (int i = 0; i < num_threads; i++) {
        double *partial_sum;
        pthread_join(threads[i], (void **)&partial_sum);
        pi += *partial_sum;
    }

    pi *= 4.0;
    printf("Calculated Pi: %.15f\n", pi);
    return 0;
}
