#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#define STEPS_COUNT 200000000

typedef struct {
    int start_compute_index, end_compute_index;
} ComputingRange;

void* compute(void* arg) {
    ComputingRange* range = (ComputingRange*) arg;
    double patrition_sum = 0;
    for (int i = range->start_compute_index; 
            i < range->end_compute_index; i++) {
        patrition_sum += ((i % 2) ? (-1) : 1) / (((double) 2) * i + 1);
    }
#ifdef DEBUG
    printf("%8d -> %8d: %.15g\n", range->start_compute_index, 
            range->end_compute_index, patrition_sum); 
#endif
    double* result = malloc(sizeof(double));
    *result = patrition_sum;
    return result;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Invalid arguments count\n");
        exit(EXIT_FAILURE);
    }

    long threads_count = strtol(argv[1], NULL, 10);
    long computing_range_size = STEPS_COUNT / threads_count;
    ComputingRange* compt_ranges = 
        (ComputingRange*) malloc(sizeof(ComputingRange) * threads_count);
    pthread_t* thread_ids = (pthread_t*) malloc(sizeof(pthread_t) * threads_count);

    for (int i = 0; i < threads_count; i++) {
        compt_ranges[i].start_compute_index = i * computing_range_size;
        compt_ranges[i].end_compute_index = (i == threads_count - 1)
            ? STEPS_COUNT : ((i + 1) * computing_range_size);
        pthread_create(&thread_ids[i], NULL, compute, &compt_ranges[i]);
#ifdef DEBUG
        printf("thread %ld was created\n", thread_ids[i]);
#endif
    }

    double result = 0;
    double* thread_result = NULL;

    for (int i = 0; i < threads_count; i++) {
        pthread_join(thread_ids[i], (void**) &thread_result);
        result += *thread_result;
        free(thread_result);
    }
    
    printf("Result of computations: %.15g\n", result * 4);

    free(compt_ranges);
    exit(EXIT_SUCCESS);
}
