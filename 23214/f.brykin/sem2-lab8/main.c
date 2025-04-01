#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define num_steps 200000000

typedef struct {
    int start;
    int end;
    double partial_sum;
} ThreadData;

void* calculate_pi_part(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    double sum = 0.0;
    for (int i = data->start; i < data->end; i++) {
        sum += 1.0 / (i * 4.0 + 1.0);
        sum -= 1.0 / (i * 4.0 + 3.0);
    }
    data->partial_sum = sum;
    pthread_exit(NULL);
}

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <number of threads>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int num_threads = atoi(argv[1]);
    if (num_threads <= 0) {
        fprintf(stderr, "Number of threads must be positive\n");
        return EXIT_FAILURE;
    }

    pthread_t threads[num_threads];
    ThreadData thread_data[num_threads];
    int steps_per_thread = num_steps / num_threads;

    for (int i = 0; i < num_threads; i++) {
        if (i == 0) {
            thread_data[i].start = 0;
        } else {
            thread_data[i].start = thread_data[i - 1].end;
        }
        if (i < num_steps % num_threads) {
            thread_data[i].end = thread_data[i].start + steps_per_thread + 1;
        } else {
            thread_data[i].end = thread_data[i].start + steps_per_thread;
        }
        pthread_create(&threads[i], NULL, calculate_pi_part, &thread_data[i]);
    }

    double pi = 0.0;
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
        pi += thread_data[i].partial_sum;
    }

    pi *= 4.0;
    printf("pi done - %.15g \n", pi);

    return EXIT_SUCCESS;
}
