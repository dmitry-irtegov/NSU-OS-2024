#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define num_steps 200000000
int num_of_threads;

typedef struct thread_struct {
 	int thread_id;
 	double thread_sum;
} thread_struct;

void* thread_func(void* args) {
    double pi = 0;
    thread_struct* data = (thread_struct*) args;
    for (int i = data->thread_id; i < num_steps; i += num_of_threads) {
        pi += 1.0 / (i * 4.0 + 1.0);
        pi -= 1.0 / (i * 4.0 + 3.0);
    }
    data->thread_sum = pi;
    pthread_exit((data));
}

int main(int argc, char** argv) {
    double pi = 0;

    if (argc != 2) {
        fprintf(stderr, "Error: wrong num of arguments\n");
 		exit(EXIT_FAILURE);
    }

    num_of_threads = atoi(argv[1]);
    if (num_of_threads < 1) {
        fprintf(stderr, "Error: too few threads\n");
 		exit(EXIT_FAILURE);
    }

    pthread_t *threads = malloc(num_of_threads * sizeof(pthread_t));
    thread_struct *args = malloc(num_of_threads * sizeof(thread_struct));

    for (int i = 0; i < num_of_threads; i++) {
        args[i].thread_id = i;
        pthread_create(&threads[i], NULL, thread_func, (void*)&args[i]);
    }

    for (int i = 0; i < num_of_threads; i++) {
        thread_struct* result;
        pthread_join(threads[i], (void **)&result);
        pi += result->thread_sum;
    }
    pi = pi * 4.0;
    printf("pi done: %.15g \n", pi);

    free(args);
    free(threads);
    exit(EXIT_SUCCESS);
}