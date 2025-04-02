#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define num_steps 200000000

typedef struct {
    long long thread_id;
    double partial_sum;
} thread_data;

int nthreads;

void *calculate(void *param) {
    thread_data *data = (thread_data *)param;
    double localpi = 0.0;
    long long i;

    for (i = data->thread_id; i < num_steps; i += nthreads) {
        localpi += 1.0 / (i * 4.0 + 1.0);
        localpi -= 1.0 / (i * 4.0 + 3.0);
    }

    data->partial_sum = localpi;
    pthread_exit(NULL);
}

int main(int argc, char **argv) {
    double pi = 0.0;
    int i;
    pthread_t *ids;
    thread_data *params;

    if (argc != 2) {
        fprintf(stderr, "usage: %s threadnum\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    nthreads = atoi(argv[1]);
    if (nthreads < 1) {
        fprintf(stderr, "Number of threads must be at least 1.\n");
        exit(EXIT_FAILURE);
    }

    params = malloc(nthreads * sizeof(thread_data));
    ids = malloc(nthreads * sizeof(pthread_t));

    for (i = 0; i < nthreads; i++) {
        params[i].thread_id = i;
        pthread_create(&ids[i], NULL, calculate, (void *)&params[i]);
    }

    for (i = 0; i < nthreads; i++) {
        pthread_join(ids[i], NULL);
        pi += params[i].partial_sum;
    }

    pi *= 4.0;
    printf("pi = %.16f\n", pi);

    free(params);
    free(ids);

    return EXIT_SUCCESS;
}

