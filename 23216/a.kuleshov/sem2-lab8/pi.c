#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define num_steps 200000000

typedef struct {
    int thread_id;
    int num_threads;
} thread_data_t;

void *compute_pi(void *arg) {
    thread_data_t *data = arg;
    int id = data->thread_id;
    int num_threads = data->num_threads;

    double *partial_sum = malloc(sizeof(double));
    if (partial_sum == NULL) {
        perror("malloc");
        pthread_exit(NULL);
    }

    *partial_sum = 0.0;

    for (int i = id; i < num_steps; i += num_threads) {
        *partial_sum += 1.0 / (i * 4.0 + 1.0);
        *partial_sum -= 1.0 / (i * 4.0 + 3.0);
    }

    pthread_exit(partial_sum);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <num_threads>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int num_threads = atoi(argv[1]);
    if (num_threads <= 0) {
        fprintf(stderr, "Number of threads must be > 0\n");
        exit(EXIT_FAILURE);
    }

    pthread_t threads[num_threads];
    thread_data_t thread_data[num_threads];

    for (int i = 0; i < num_threads; i++) {
        thread_data[i].thread_id = i;
        thread_data[i].num_threads = num_threads;
        if (pthread_create(&threads[i], NULL, compute_pi, &thread_data[i]) != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }

    double pi = 0.0;
    for (int i = 0; i < num_threads; i++) {
        double *partial_sum;
        if (pthread_join(threads[i], (void **)&partial_sum) != 0) {
            perror("pthread_join");
            exit(EXIT_FAILURE);
        }
        pi += *partial_sum;
        free(partial_sum);
    }

    pi *= 4.0;

    printf("pi done - %.15g \n", pi);

    return EXIT_SUCCESS;
}
