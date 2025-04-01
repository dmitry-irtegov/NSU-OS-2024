#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

int nthreads = 0;
int num_steps = 1000000;
int stop = 0;
int max_iterations = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER;
pthread_barrier_t barrier;

typedef struct pi_pthread{
    int index;
    double partial_sum;
    int iterations;
} pi_calculate;

void* calculate(void *param){
    double localpi = 0.0;
    pi_calculate* calculating_thread = (pi_calculate*)param; 
    long long i= calculating_thread->index;
    calculating_thread->iterations = 0;

    while (!stop) {
        for (i;; i += nthreads) {
            localpi += 1.0/(i*4.0 + 1.0);
            localpi -= 1.0/(i*4.0 + 3.0);
            calculating_thread->iterations++;
            if (calculating_thread->iterations % num_steps == 0) {
                break;
            }
        }
    }

    pthread_mutex_lock(&mutex);
    if (calculating_thread->iterations > max_iterations) {
        max_iterations = calculating_thread->iterations;
    }
    pthread_mutex_unlock(&mutex);
    
    pthread_barrier_wait(&barrier);
    if (calculating_thread->iterations < max_iterations) {
        for (i;; i += nthreads) {
            localpi += 1.0/(i*4.0 + 1.0);
            localpi -= 1.0/(i*4.0 + 3.0);
            calculating_thread->iterations++;
            if (calculating_thread->iterations >= max_iterations) {
                break;
            }
        }
    }
    calculating_thread->partial_sum = localpi;
    return param;
}

void sigint_handler(int sig) {
    stop = 1;
}

int main(int argc, char** argv) {
    double pi = 0;
    int i = 0;
    pthread_t* ids;
    pi_calculate* threads;
    if (argc > 1) {
        nthreads = atoi(argv[1]);
    }
    if (nthreads < 1) {
        fprintf(stderr, "usage: %s threadnum\n", argv[0]);
        exit(0);
    }

    ids = (pthread_t*) malloc(sizeof(pthread_t) * nthreads);
    threads = (pi_calculate*) malloc(sizeof(pi_calculate) * nthreads);
    signal(SIGINT, sigint_handler);
    int code;
    code = pthread_barrier_init(&barrier, NULL, nthreads);
    if (code != 0) {
        fprintf(stderr, "pthread_barrier_init error %s\n", strerror(code));
        exit(1);
    }
    for (i = 0; i < nthreads; i++) {
        threads[i].index = i;
        code = pthread_create(&(ids[i]), NULL, calculate, (void*)&threads[i]);
        if (code != 0) {
            fprintf(stderr, "pthread_create error %s\n", strerror(code));
            exit(1);
        }
    }
    pi_calculate* res;
    for (i = 0; i < nthreads; i++) {
        pthread_join(ids[i], (void**)&res);
        pi += ((pi_calculate*)res)->partial_sum;
    }
    pi *= 4.0;
    printf("pi = %.16f\n", pi);
    free(ids);
    free(threads);
    return(0);
}
