#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#define NUM_STEPS 200000000

typedef struct {
    int index;
    double partial_sum;
} thread_data;

int nthreads;

void* calculate(void* param) {
    double partial_pi = 0.0;
    thread_data* data = (thread_data*)param;
    for (int i = data->index; i < NUM_STEPS ; i += nthreads) {        
        partial_pi += 1.0 / (i * 4.0 + 1.0);
        partial_pi -= 1.0 / (i * 4.0 + 3.0);
    }
    data->partial_sum = partial_pi;
    return data;
}

int main(int argc, char** argv) {
    if (argc > 1) {
        nthreads = atol(argv[1]);
    }
    if (nthreads < 1 || nthreads > 100) {
        fprintf(stderr, "Invalid thread count (1-100)\n");
        exit(EXIT_FAILURE);
    }

    int rc;
    pthread_t* ids = (pthread_t*)malloc(nthreads * sizeof(pthread_t));
    thread_data* data = (thread_data*)malloc(nthreads * sizeof(thread_data));
    double pi = 0.0;

    for (int i = 0; i < nthreads; i++) {
        data[i].index = i;
        data[i].partial_sum = 0;
        if ((rc = pthread_create(&ids[i], NULL, calculate, data + i)) != 0) {
            fprintf(stderr, "%d: creating: %s\n", i, strerror(rc));
            exit(EXIT_FAILURE);
        }
    }
    
    for (int i = 0; i < nthreads; i++) {
        thread_data* res; 
        pthread_join(ids[i], (void**)&res);
        pi += res->partial_sum;
    }
    
    pi *= 4.0;
    printf("pi done - %.15g \n", pi);    
    exit(EXIT_SUCCESS);
}
