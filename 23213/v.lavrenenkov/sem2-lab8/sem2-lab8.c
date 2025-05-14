#include <stdio.h>
#include <stdlib.h> 
#include <pthread.h>
#include <errno.h>
#include <string.h>
#define NUM_STEPS 200000000

typedef struct {
    int start;
    double partial_pi;
} ThreadData;
int num_threads;


void* counting_thread(void * val) {
    ThreadData* th = (ThreadData*) val;
    double tmp = 0.0; 
    for (int i = th->start; i < NUM_STEPS; i+=num_threads) {
        tmp += 1.0/(i*4.0 + 1.0);
        tmp -= 1.0/(i*4.0 + 3.0);
    }
    th->partial_pi = tmp;
    return val;
}
int main(int argc, char** argv) {
    if(argc != 2) {
        fprintf(stderr, "Usage: %s number_of_threads \n", argv[0]);
        return 1;
    }
    num_threads = atoi(argv[1]);
    if (num_threads <= 0) {
        fprintf(stderr, "Wrong number of threads\n");
        return 1;
    }
    double pi = 0;
    pthread_t threads[num_threads];
    ThreadData th_data[num_threads];
    for (int i = 0; i < num_threads; i++) {
        int err = 0;
        th_data[i].start = i;
        th_data[i].partial_pi = 0;
        if ((err = pthread_create(&threads[i], NULL, counting_thread, (void *)&th_data[i])) != 0) {
            fprintf(stderr, "Couldn`t open thread: %s \n", strerror(err));
            return 1;
        }
    }
    for (int i = 0; i < num_threads; i++) {
        void * ret_val;
        pthread_join(threads[i], &ret_val);
        pi += ((ThreadData *) ret_val)->partial_pi;
    }
    pi = pi * 4.0;
    printf("pi done - %.15g \n", pi);    
    return 0;
}
