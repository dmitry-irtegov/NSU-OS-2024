#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_STEPS 200000000

typedef struct {
    long long from;
    long long to;
} thread_arg;

void *thread_function(void *arg) {
    double *pi_part = calloc(1, sizeof(double));
    if (!pi_part) {
        perror("calloc failed");
        return NULL;
    }
    thread_arg *arguments = (thread_arg*)arg;
    for (long long i = arguments->from; i < arguments->to; i++) {
        *pi_part += 1.0 / (i * 4.0 + 1.0);
        *pi_part -= 1.0 / (i * 4.0 + 3.0);
    }
    return (void*)pi_part;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <threads_number>.\n", argv[0]);
        return EXIT_FAILURE;
    }

    int threads_number = atoi(argv[1]);
    if (threads_number <= 0) {
        fprintf(stderr, "Invalid threads_number.\n");
        return EXIT_FAILURE;
    }

    pthread_t *threads = malloc(sizeof(pthread_t) * threads_number);
    if (!threads) {
        perror("malloc threads failed");
        return EXIT_FAILURE;
    }

    thread_arg *args = malloc(sizeof(thread_arg) * threads_number);
    if (!args) {
        perror("malloc args failed");
        free(threads);
        return EXIT_FAILURE;
    }

    long long step = NUM_STEPS / threads_number;
    long long remainder = NUM_STEPS % threads_number;
    long long current = 0;

    for (int i = 0; i < threads_number; i++) {
        args[i].from = current;
        args[i].to = current + step + (i < remainder ? 1 : 0);
        current = args[i].to;
        int code = pthread_create(&threads[i], NULL, thread_function, &args[i]);
        if (code != 0) {
            fprintf(stderr, "Failed to create thread %d: %d\n", i + 1, code);
            free(threads);
            free(args);
            return EXIT_FAILURE;
        }
    }

    double pi = 0.0;
    void *part_sum;
    for (int i = 0; i < threads_number; i++) {
        int code = pthread_join(threads[i], &part_sum);
        if (code != 0) {
            fprintf(stderr, "Failed to join thread %d: %d\n", i + 1, code);
            free(threads);
            free(args);
            return EXIT_FAILURE;
        }
        if (part_sum == NULL) {
            free(threads);
            free(args);
            return EXIT_FAILURE;
        }
        pi += *((double*)part_sum);
        free(part_sum);
    }

    pi *= 4.0;
    printf("pi done - %.15g\n", pi);

    free(threads);
    free(args);
    return EXIT_SUCCESS;
}
