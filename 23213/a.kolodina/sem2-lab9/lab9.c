#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#define NUM_STEPS 1000000

int nthreads = 0;
int stop = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_barrier_t barrier;

typedef struct pi_pthread{
    int index;
    double partial_sum;
} pi_calculate;

void* calculate(void *param){
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGINT);
    pthread_sigmask(SIG_BLOCK, &sigset, NULL);
    
    pi_calculate* calculating_thread = (pi_calculate*)param; 
    double localpi = 0.0;
    int i = calculating_thread->index;
    int iterations = 0;
    
    for (i;; i += nthreads) {
        localpi += 1.0/(i*4.0 + 1.0);
        localpi -= 1.0/(i*4.0 + 3.0);
        iterations++;
        if (iterations % NUM_STEPS == 0) {
            pthread_barrier_wait(&barrier);
            if (stop) {
                break;
            }
        }
    }
    printf("iterations = %d\n", iterations);
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
    if (ids == NULL) {
        perror("malloc error");
        exit(1);
    }
    threads = (pi_calculate*) malloc(sizeof(pi_calculate) * nthreads);
    if (threads == NULL) {
        perror("malloc error");
        exit(1);
    }
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
    return 0;
}
