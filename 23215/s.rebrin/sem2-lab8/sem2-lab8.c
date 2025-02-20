#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>

#define num_steps 200000000

typedef struct {    
    long long start;        
    long long end;          
    double partial_sum;     
} thread_data_t;

int min(int a, int b) {
    return a < b ? a : b;
}

void* compute_partial_sum(void* arg) {
    thread_data_t* data = (thread_data_t*)arg;
    double sum = 0.0;
    for (long long i = data->start;i < data->end;i++) {
        sum += 1.0 / ((double)i * 4.0 + 1.0);
        sum -= 1.0 / ((double)i * 4.0 + 3.0);
    }
    data->partial_sum = sum;
    pthread_exit(0);
}

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "%s <number_of_threads>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int num_threads = atoi(argv[1]);
    if (num_threads <= 0) {
        fprintf(stderr, "Number of threads must be positive.\n");
        return EXIT_FAILURE;
    }

    if (num_threads > num_steps) {
        num_threads = num_steps;
    }

    long long size = num_steps / num_threads;

    pthread_t* threads = malloc(num_threads * sizeof(pthread_t));

    thread_data_t* thread_data = malloc(num_threads * sizeof(thread_data_t));


    for (int i = 0;i < num_threads;i++) {
        thread_data[i].start = i * size;
        thread_data[i].end = min(i * size + size - 1, num_steps);
        if (pthread_create(&threads[i], NULL, compute_partial_sum, (void*)&thread_data[i])) {
            fprintf(stderr, "Error creating thread %d\n", i);
            for (int j = 0; j < i; j++) {
                pthread_join(threads[j], NULL);
            }
            free(threads);
            free(thread_data);
            return EXIT_FAILURE;
        }
    }

    double pi = 0.0;
    for (int i = 0;i < num_threads;i++) {
        pthread_join(threads[i], NULL);
        //printf("%g\n", thread_data[i].partial_sum);
        pi += thread_data[i].partial_sum;
    }
    pi *= 4.0;

    printf("pi: %.15g\n", pi);

    free(threads);
    free(thread_data);

    return EXIT_SUCCESS;
}