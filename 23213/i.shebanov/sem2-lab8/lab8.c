#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>

#define num_steps 200000000

int nthreads;

typedef struct CalculationJob {
    long long index;
    double partial_sum;   
    pthread_t thread;    
} CalculationJob;


void * calculation(void * arg) {
    double partial = 0.0;

    CalculationJob * job = (CalculationJob*)arg;

    for (long long i = job->index; i < num_steps; i+=nthreads) {
        partial += 1.0 / (i * 4.0 + 1.0);
        partial -= 1.0 / (i * 4.0 + 3.0);
    }

    job->partial_sum = partial;

    return NULL;
}

int main(int argc, char ** argv) {
    if(argc != 2) {
        fprintf(stderr, "You have to specify number of threads\n");
        exit(EXIT_FAILURE);
    }

    nthreads = atoi(argv[1]);
    

    double pi = 0.0;
    CalculationJob * calculations = (CalculationJob*)malloc(nthreads * sizeof(CalculationJob));

    for(int i = 0; i < nthreads; i++) {
        calculations[i].index = i;
        if(pthread_create(&calculations[i].thread, NULL, calculation, (void*)(calculations + i))) {
            fprintf(stderr, "Error while creating %d thread: %s\n", i, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    for(int i = 0; i < nthreads; i++) {
        pthread_join(calculations[i].thread, NULL);
        pi += calculations[i].partial_sum;
    }
    pi *=4;

    printf("pi done - %.15g\n", pi);

    exit(EXIT_SUCCESS);
}
