#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_STEPS 200000000

typedef struct {
    int start;
    int end;
    double partial_sum;
} thread_data;

void* calculate_partial_pi(void* arg) {
    thread_data* data = (thread_data*)arg;
    double sum = 0.0;
    
    for (int i = data->start; i < data->end; i++) {
        sum += 1.0 / (i * 4.0 + 1.0);
        sum -= 1.0 / (i * 4.0 + 3.0);
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
    pthread_t threads[num_threads];
    thread_data thread_args[num_threads];
    int steps_per_thread = NUM_STEPS / num_threads;

    for (int i = 0; i < num_threads; i++) {
        thread_args[i].start = i * steps_per_thread;
        thread_args[i].end = (i == num_threads - 1) ? NUM_STEPS : (i + 1) * steps_per_thread;
        
        pthread_create(&threads[i], NULL, calculate_partial_pi, &thread_args[i]);
    }

    double pi = 0.0;
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
        pi += thread_args[i].partial_sum;
    }

    pi = pi * 4.0;
    printf("pi: %.15g \n", pi);
    return EXIT_SUCCESS;
}