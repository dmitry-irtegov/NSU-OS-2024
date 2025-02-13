#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define NUM_STEPS 200000000LL

typedef struct {
    long long start;
    long long end;
} thread_args_t;

void *compute_pi(void *arg) {
    thread_args_t *targs = (thread_args_t *) arg;
    double partial_sum = 0.0;
    for (long long i = targs->start; i < targs->end; i++) {
        if ((i & 1LL) == 0) {
            partial_sum += 1.0 / (2 * i + 1);
        } else {
            partial_sum -= 1.0 / (2 * i + 1);
        }
    }
    double *result = malloc(sizeof(double));
    if (!result) {
        perror("malloc error");
        pthread_exit(NULL);
    }
    *result = partial_sum;
    pthread_exit(result);
}

int main(int argc, char *argv[]) {
    int num_threads;
    char buf[256];
    int err_code;
    if (argc != 2) {
        fprintf(stderr, "Specify the required number of threads when starting the program\n");
        exit(-1);
    }
    num_threads = atoi(argv[1]);
    if (num_threads < 1) {
        fprintf(stderr, "Error: number_of_threads must be at least 1\n");
        exit(-1);
    }
    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
    if (!threads) {
        perror("malloc error for threads");
        exit(-1);
    }
    thread_args_t *targs = malloc(num_threads * sizeof(thread_args_t));
    if (!targs) {
        perror("malloc error for thread arguments");
        free(threads);
        exit(-1);
    }
    long long steps_per_thread = NUM_STEPS / num_threads;
    long long remainder = NUM_STEPS % num_threads;
    for (int i = 0; i < num_threads; i++) {
        targs[i].start = i * steps_per_thread + (i < remainder ? i : remainder);
        targs[i].end = targs[i].start + steps_per_thread + (i < remainder ? 1 : 0);
    }
    for (int i = 0; i < num_threads; i++) {
        if ((err_code = pthread_create(&threads[i], NULL, compute_pi, (void *)&targs[i])) != 0) {
            strerror_r(err_code, buf, sizeof(buf));
            fprintf(stderr, "Error with creating thread: %s\n", buf);
            free(threads);
            free(targs);
            exit(-1);
        }
    }
    double sum = 0.0;
    for (int i = 0; i < num_threads; i++) {
        void *thread_result;
        if ((err_code = pthread_join(threads[i], &thread_result)) != 0) {
            strerror_r(err_code, buf, sizeof(buf));
            fprintf(stderr, "Error with join %d thread: %s\n", i, buf);
            free(threads);
            free(targs);
            exit(-1);
        }
        if (thread_result == NULL) {
            fprintf(stderr, "Thread %d did not return a valid result\n", i);
            free(threads);
            free(targs);
            exit(-1);
        }
        double *partial = (double *)thread_result;
        sum += *partial;
        free(partial);
    }
    double pi = 4.0 * sum;
    printf("Computed pi = %.15g\n", pi);
    free(threads);
    free(targs);
    return 0;
}
