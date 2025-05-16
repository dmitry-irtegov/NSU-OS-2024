#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct {
    long long startIndex;
    double partialSum;
} thread_struct;

#define NUMBER_OF_STEPS 200000000

int numberOfThreads;

void *calculate(void *param)
{
    double curThreadPi = 0.0;
    long long start = ((thread_struct*) param)->startIndex;
    for (long long i = start; i < NUMBER_OF_STEPS; i += numberOfThreads) {
        curThreadPi += 1.0 / (i * 4.0 + 1.0);
        curThreadPi -= 1.0 / (i * 4.0 + 3.0);
    }
    ((thread_struct *)param) -> partialSum = curThreadPi;
    return param;
}

int main(int argc, char **argv)
{
    if (argc >= 2)
        numberOfThreads = atol(argv[1]);
    else {
        fprintf(stderr, "Empty number of threads\n");
        exit(EXIT_FAILURE);
    }

    if (numberOfThreads < 1) {
        fprintf(stderr, "Invalid number of threads");
        exit(EXIT_FAILURE);
    }
    double pi = 0;
    pthread_t *ids;
    thread_struct *params;

    params = malloc(numberOfThreads * sizeof(thread_struct));
    ids = malloc(numberOfThreads * sizeof(pthread_t));

    for (int i = 0; i < numberOfThreads; i++) {
        params[i].startIndex = i;
        if (pthread_create(&ids[i], NULL, calculate, (void*) &params[i])) {
            fprintf(stderr, "[pthread_create]: Return 0");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < numberOfThreads; i++) {
        thread_struct *res;
        int retval = pthread_join(ids[i], (void **)&res);
        if (retval == 0){
            pi += res->partialSum;
        } else {
            fprintf(stderr, "[pthread_join] Return Error:");
            exit (EXIT_FAILURE);
        }
    }

    pi *= 4.0;
    printf("Result of calculation is %.16f\n", pi);

    return (EXIT_SUCCESS);
}