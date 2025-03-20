#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>

#define NUM_STEPS 200000000

int steps_per_thread;

void* pi_calculation(void *param) {
    int start = (*(int *)param) * steps_per_thread;
    double *partial_sum = (double *)malloc(sizeof(double));
    double part_sum = 0;

    for (int i = start; i < start + steps_per_thread; i++) {
        part_sum += 1.0/(i*4.0 + 1.0);
        part_sum -= 1.0/(i*4.0 + 3.0);
    }

    *partial_sum = part_sum;

    free(param);
    pthread_exit((void *)partial_sum);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "a gde number of threads?\n");
        exit(EXIT_FAILURE);
    }

    int number_of_threads = atoi(argv[1]);

    if (number_of_threads < 1) {
        fprintf(stderr, "bad number of threads\n");
        exit(EXIT_FAILURE);
    }

    if (number_of_threads > NUM_STEPS) {
        number_of_threads = NUM_STEPS;
    }

    printf("number_of_threads: %d\n", number_of_threads);

    steps_per_thread = NUM_STEPS / number_of_threads;
    pthread_t *threads = (pthread_t *)malloc(sizeof(pthread_t) * number_of_threads);

    for (int i = 0; i < number_of_threads; i++) {
        int *p = (int *)malloc(sizeof(int));
        *p = i;
        if (pthread_create(&threads[i], NULL, pi_calculation, (void *)p) != 0) {
            fprintf(stderr, "can't create thread\n");
            exit(EXIT_FAILURE);
        }
    }

    double pi = 0;
    double *part_sum;

    for (int i = 0; i < number_of_threads; i++) {
        pthread_join(threads[i], (void **)&part_sum);
        pi += *(double *)part_sum;
        free(part_sum);
    }

    pi *= 4.0;
    printf("pi done - %.15g \n", pi); 

    free(threads);

    exit(EXIT_SUCCESS);
}
