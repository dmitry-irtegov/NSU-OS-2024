#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <stdint.h>

int nthreads;
int8_t stop_calc = 0;
int8_t isFinished = 0;
pthread_barrier_t barrier;

typedef struct {
    int32_t index;
    double partial_sum;
} thread_data;

void handle_sigint(int sig) {
    stop_calc = 1;
}

void* calculate(void* param) {
    thread_data* data = (thread_data*)param;
    uint64_t i = data->index;

    for (uint64_t j = 0; ; j++) {
        data->partial_sum += 1.0 / (i * 4.0 + 1.0);
        data->partial_sum -= 1.0 / (i * 4.0 + 3.0);
        i += nthreads;

        if (j % 1000000 == 0) {
            int res = pthread_barrier_wait(&barrier);
            if (res == 0 || res == PTHREAD_BARRIER_SERIAL_THREAD) {
                if (isFinished) {
                    printf("Thread %d: runs %lu; sum %.15g\n", data->index, j, data->partial_sum);
                    pthread_exit(data);
                }
                res = pthread_barrier_wait(&barrier);
                if (res == 0 || res == PTHREAD_BARRIER_SERIAL_THREAD) {
                    if (stop_calc) {
                        isFinished = 1;
                    }
                }
            }
        }
    }
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
    if ((rc = pthread_barrier_init(&barrier, NULL, nthreads)) != 0) {
        fprintf(stderr, "Barrier init failed: %s\n", strerror(rc));
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, handle_sigint);
    double pi = 0.0;
    pthread_t* ids = malloc(nthreads * sizeof(pthread_t));
    thread_data* data = malloc(nthreads * sizeof(thread_data));

    for (int i = 0; i < nthreads; i++) {
        data[i].index = i;
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

    if ((rc = pthread_barrier_destroy(&barrier)) != 0) {
        fprintf(stderr, "Barrier destroy failed: %s\n", strerror(rc));
        exit(EXIT_FAILURE);
    }

    pi = pi * 4.0;
    printf("pi done - %.15g \n", pi);
    exit(EXIT_SUCCESS);
}
