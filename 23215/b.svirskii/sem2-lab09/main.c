#ifndef __sun
#define _GNU_SOURCE
#include <features.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#define THREAD_STEPS_COUNT 1000000


pthread_spinlock_t get_step_lock;

unsigned long long next_available_step = 0;
unsigned char is_computation_ended = 0;

unsigned long long get_next_step() {
    unsigned long long prev_available_step;

    pthread_spin_lock(&get_step_lock);
    
    prev_available_step = next_available_step;
    next_available_step += THREAD_STEPS_COUNT;
    
    if (next_available_step < prev_available_step) {
        is_computation_ended = 1;
    }
    
    pthread_spin_unlock(&get_step_lock);
    
    return prev_available_step;
}

void* compute(void* arg) {
    double patrition_sum = 0;
    while (!is_computation_ended) {
        unsigned long long start_compute_step = get_next_step();
        unsigned long long end_compute_step = 
            start_compute_step + THREAD_STEPS_COUNT;
        for (unsigned long long i = start_compute_step; 
                i < end_compute_step; i++) {
            patrition_sum += ((i % 2) ? (-1) : 1) / (((double) 2) * i + 1);
        }
#ifdef DEBUG
        printf("%llu -> %llu: %.15g\n", start_compute_step, 
                end_compute_step, patrition_sum); 
#endif
    }
    double* result = (double*) malloc(sizeof(double));
    *result = patrition_sum;
    return result;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Invalid arguments count\n");
        exit(EXIT_FAILURE);
    }


    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigprocmask(SIG_BLOCK, &set, NULL);

    pthread_spin_init(&get_step_lock, PTHREAD_PROCESS_PRIVATE);

    long threads_count = strtol(argv[1], NULL, 10);
    pthread_t* threads = (pthread_t*) malloc(sizeof(pthread_t) * threads_count);

    for (int i = 0; i < threads_count; i++) {
        pthread_create(&threads[i], NULL, compute, NULL);
#ifdef DEBUG
        printf("thread %ld was created\n", threads[i]);
#endif
    }


#ifdef __sun
    sigwait(&set);
#else
    int sig;
    sigwait(&set, &sig);
#endif

    is_computation_ended = 1;

    double result = 0;
    double* thread_result = NULL;

    for (int i = 0; i < threads_count; i++) {
        pthread_join(threads[i], (void**) &thread_result);
        result += *thread_result;
        free(thread_result);
    }
    
    printf("Result of computations: %.15g\n", result * 4);

    pthread_spin_destroy(&get_step_lock);
    free(threads);
    exit(EXIT_SUCCESS);
}

