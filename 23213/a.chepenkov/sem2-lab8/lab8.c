#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define num_steps 200000000

int nthreads;

typedef struct ComputationData {
    long long index;
    double partial_sum;       
} ComputationData;

void* calculate(void* param) {
    double partial_sum = 0.0;

    long long i = ((ComputationData*)param)->index;

    for (; i < num_steps; i += nthreads) {
        partial_sum += 1.0 / (i * 4.0 + 1.0);
        partial_sum -= 1.0 / (i * 4.0 + 3.0);
    }
    fprintf(stderr, "Thread %lld finished, partial sum %.16f\n",
        ((ComputationData*)param)->index, partial_sum);

    ((ComputationData*)param)->partial_sum = partial_sum;
    return param;
}

int main(int argc, char** argv) {
    if (argc > 1) {
        nthreads = atol(argv[1]);
    }
    if (nthreads < 1) {
        fprintf(stderr, "You have to specify number of threads\n");
        exit(EXIT_FAILURE);
    }

    double pi = 0.0;
    int err;
    int flag = 1;
    ComputationData* params = malloc(nthreads * sizeof(ComputationData));
    pthread_t* ids = malloc(nthreads * sizeof(pthread_t));

    for (int i = 0; i < nthreads && flag; i++) {
        params[i].index = i;
        err = pthread_create(&ids[i], NULL, calculate, (void*)(params + i));
        if (err != 0) {
            fprintf(stderr, "Error while creating %d thread: %s\n", i, strerror(err));
            flag = 0;
        }
    }

    for (int i = 0; i < nthreads && flag; i++) {
        ComputationData* res;
        err = pthread_join(ids[i], (void**)&res);
        if (err != 0) {
            fprintf(stderr, "Error while joining %d thread: %s\n", i, strerror(err));
            flag = 0;
        }
        pi += res->partial_sum;
    }

    free(params);
    free(ids);

    if (!flag) {
        return EXIT_FAILURE;
    }

    pi *= 4.0;
    printf("pi = %.16f\n", pi);

  
    return EXIT_SUCCESS;
}
