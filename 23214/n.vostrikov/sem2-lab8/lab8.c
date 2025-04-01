#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_ITERATIONS 1000000000

typedef struct {
    int thread_id;
    int num_threads;
    double partial_sum;
} ThreadData;

void *compute_pi(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    int start = data->thread_id;
    int step = data->num_threads;
    double sum = 0.0;

    for (int i = start; i < NUM_ITERATIONS; i += step) {
        double term = 1.0 / (2 * i + 1);
        sum += (i % 2 == 0) ? term : -term;
    }

    data->partial_sum = sum;
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "invalid number of arguments\n");
        exit(1);
    }

    int num_threads = atoi(argv[1]);
    if (num_threads <= 0) {
        fprintf(stderr, "number of threads must be positive.\n");
        exit(2);
    }

    pthread_t* threads = malloc(num_threads * sizeof(pthread_t));
    if(threads == NULL) {
        perror("malloc failed");
        exit(3);
    }
    ThreadData* thread_data = malloc(num_threads * sizeof(ThreadData));
    if(thread_data == NULL) {
        perror("malloc failed");
        exit(4);
    }

    for (int i = 0; i < num_threads; i++) {
        thread_data[i].thread_id = i;
        thread_data[i].num_threads = num_threads;
        thread_data[i].partial_sum = 0.0;

        if (pthread_create(&threads[i], NULL, compute_pi, (void*)(thread_data + i)) != 0) {
            perror("pthread_create failed");
            free(threads);
            free(thread_data);
            exit(5);
        }
    }

    double pi = 0.0;
    for (int i = 0; i < num_threads; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("pthread_join failed");
            free(threads);
            free(thread_data);
            exit(6);
        }
        pi += thread_data[i].partial_sum;
    }
    pi *= 4.0; 
    printf("pi = %.15f\n", pi);
    free(threads);
    free(thread_data);
    exit(0);
}