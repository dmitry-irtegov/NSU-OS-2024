#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define num_steps 200000000

long num_threads;

union thread_arg {
    long i;
    double result;
};

struct thread_data {
    pthread_t thread;
    union thread_arg arg;
};

void *thread_func(void *void_arg) {
    union thread_arg *arg = (union thread_arg *)void_arg;

    double pi = 0;
    for (long i = arg->i; i < num_steps; i += num_threads) {
        pi += 1.0/(i*4.0 + 1.0);
        pi -= 1.0/(i*4.0 + 3.0);
    }

    arg->result = pi;
    
    return void_arg;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: lab8 <num_threads>\n");
        return 1;
    }
    
    num_threads = strtol(argv[1], NULL, 10);
    if (num_threads <= 0) {
        fprintf(stderr, "Invalid number of threads '%s': must be a positive integer\n", argv[1]);
        return 1;
    }
    
    struct thread_data *threads = malloc(num_threads * sizeof(struct thread_data));
    if (threads == NULL) {
        perror("Not enough memory");
        return 1;
    }

    for (long i = 0; i < num_threads; i++) {
        threads[i].arg.i = i;

        int thread_error = pthread_create(&threads[i].thread, NULL, thread_func, (void *)&threads[i].arg);
        if (thread_error != 0) {
            fprintf(stderr, "Failed to create thread: %s\n", strerror(thread_error));
            return 1;
        }
    }

    double pi = 0;

    for (long i = 0; i < num_threads; i++) {
        union thread_arg *ret;
        
        int join_error = pthread_join(threads[i].thread, (void **)&ret);
        if (join_error != 0) {
            fprintf(stderr, "Failed to join thread: %s\n", strerror(join_error));
            return 1;
        }
        
        pi += ret->result;
    }
    
    pi = pi * 4.0;
    printf("pi done - %.15g\n", pi);
    
    return 0;
}
