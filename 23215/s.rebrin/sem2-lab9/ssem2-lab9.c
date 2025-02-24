#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include <signal.h>

int num_steps;

int stop = 0;

void func(int sig) {
    stop = 1;
    signal(SIGINT, func);
}

typedef struct {    
    int num_thread; 
    int count;
    double partial_sum;     
} thread_data_t;

int min(int a, int b) {
    return a < b ? a : b;
}

void* compute_partial_sum(void* arg) {
    thread_data_t* data = (thread_data_t*)arg;
    double sum = 0.0;
    int i = data->num_thread;
    while(!stop) {
        sum += 1.0 / ((double)i * 4.0 + 1.0);
        sum -= 1.0 / ((double)i * 4.0 + 3.0);
        i += data->count;
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

    int num_steps = num_threads;

    pthread_t* threads = malloc(num_threads * sizeof(pthread_t));

    thread_data_t* thread_data = malloc(num_threads * sizeof(thread_data_t));

    signal(SIGINT, func);

    for (int i = 0;i < num_threads;i++) {

        signal(SIGINT, func);
        thread_data[i].num_thread = i;
        thread_data[i].count = num_steps;
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
    
    pause();

    double pi = 0.0;
    for (int i = 0;i < num_threads;i++) {
        pthread_join(threads[i], NULL);
        //printf("%g\n", thread_data[i].partial_sum);
        pi += thread_data[i].partial_sum;
    }
    pi *= 4.0;

    printf("\npi: %.15g\n", pi);

    free(threads);
    free(thread_data);

    return EXIT_SUCCESS;
}