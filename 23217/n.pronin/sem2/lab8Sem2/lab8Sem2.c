#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>  

#ifndef NUM_STEPS
#define NUM_STEPS 200000000 
#endif

typedef struct {
    int thread_id;
    int num_threads;
} thread_arg_t;

void *compute_pi_part(void *arg) {
    thread_arg_t *targ = (thread_arg_t *)arg;
    int id = targ->thread_id;
    int num_threads = targ->num_threads;

    double *partial_sum = malloc(sizeof(double));
    if (!partial_sum) {
        perror("malloc");
        pthread_exit(NULL);
    }

    *partial_sum = 0.0;
    for (long i = id; i < NUM_STEPS; i += num_threads) {
        *partial_sum += 1.0 / (i * 4.0 + 1.0);
        *partial_sum -= 1.0 / (i * 4.0 + 3.0);
    }

    pthread_exit(partial_sum);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <num_threads>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int num_threads = atoi(argv[1]);
    if (num_threads <= 0) {
        fprintf(stderr, "Number of threads must be positive.\n");
        return EXIT_FAILURE;
    }

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);  // Start 

    pthread_t threads[num_threads];
    thread_arg_t args[num_threads];

    for (int i = 0; i < num_threads; ++i) {
        args[i].thread_id = i;
        args[i].num_threads = num_threads;
        if (pthread_create(&threads[i], NULL, compute_pi_part, &args[i]) != 0) {
            perror("pthread_create");
            return EXIT_FAILURE;
        }
    }

    double pi = 0.0;
    for (int i = 0; i < num_threads; ++i) {
        void *ret_val;
        if (pthread_join(threads[i], &ret_val) != 0) {
            perror("pthread_join");
            return EXIT_FAILURE;
        }
        double *partial = (double *)ret_val;
        pi += *partial;
        free(partial);
    }

    pi *= 4.0;

    clock_gettime(CLOCK_MONOTONIC, &end);  // End 

    double elapsed = (end.tv_sec - start.tv_sec) +
                     (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("pi done - %.15g\n", pi);
    printf("Execution time with %d threads: %.6f seconds\n", num_threads, elapsed);

    return EXIT_SUCCESS;
}
