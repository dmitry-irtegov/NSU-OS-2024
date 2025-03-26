#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define NUM_STEPS 200000000

typedef struct {
    int id;
    double part_sum;
} thread_data;

int num_threads;

void* pi_count(void *arg) {

    thread_data *data = (thread_data* )arg;
    double part_pi = 0;

    for (int i = data->id; i < NUM_STEPS; i += num_threads) {
        part_pi += 1.0/(i*4.0 + 1.0);
        part_pi -= 1.0/(i*4.0 + 3.0);
    }
    data->part_sum = part_pi;
    return arg;
}
int main(int argc, char *argv[]) {

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <num_threads>\n", argv[0]);
        exit(1);
    }

    num_threads = atoi(argv[1]);
    if (num_threads <= 0) {
        fprintf(stderr, "Number of threads must be bigger then 0.\n");
        exit(1);
    }

    thread_data* threads_data = (thread_data*)malloc(num_threads * sizeof(thread_data));
    pthread_t* threads = (pthread_t*)malloc(num_threads * sizeof(pthread_t));

    for (int i = 0; i < num_threads; i++) {
        threads_data[i].id = i;
        threads_data[i].part_sum = 0;
        int res = pthread_create(&threads[i], NULL, pi_count, (void*)&threads_data[i]);
        if (res != 0) {
            fprintf(stderr, "%s: creating thread: %s\n", argv[0], strerror(res));
            exit(1);
        }  
    }

    double pi = 0;

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
        pi += threads_data[i].part_sum;
    }

    pi = pi * 4.0;
    printf("pi done - %.15g \n", pi);    
    return 0;
}
