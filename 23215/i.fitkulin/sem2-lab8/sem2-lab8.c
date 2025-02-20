#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

#define num_steps 200000000

typedef struct thread_args {
    long long tid;
    double partial_sum;
} thread_args;

int threads_num;

void *calculate_pi(void *arg) {
    double partial_pi = 0.0;
    long long i;

    for (i = ((thread_args*)arg)->tid; i < num_steps; i += threads_num) {
        partial_pi += 1.0 / (i * 4.0 + 1.0);
        partial_pi -= 1.0 / (i * 4.0 + 3.0);
    }
    ((thread_args*)arg)->partial_sum = partial_pi;
    return arg;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Specify the number of threads\n");
        exit(EXIT_FAILURE);
    }

    threads_num = atoi(argv[1]);
    if (threads_num < 1) {
        fprintf(stderr, "Specify the number of threads not less than 1\n");
        exit(EXIT_FAILURE);
    }
    
    double pi = 0;
    pthread_t *ids = malloc(threads_num * sizeof(pthread_t));
    thread_args *params = malloc(threads_num * sizeof(thread_args));
    int i = 0;
    for (i = 0; i < threads_num; i++) {
        params[i].tid = i;
        pthread_create(&ids[i], NULL, calculate_pi, (void*)&params[i]);
    }

	for (i = 0; i < threads_num; i++) {
        thread_args* res;
        pthread_join(ids[i], (void **)&res);
        pi += res->partial_sum;
	}

    pi = pi * 4.0;
    printf("pi done - %.15g \n", pi); 
    
    free(ids);
    free(params);   
    return (EXIT_SUCCESS);
} 
