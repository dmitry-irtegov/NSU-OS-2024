#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

#define INTERVAL 10000000

typedef struct data {
    double* sum;
    int number;
    int total_threads;
} data;

int stop_flag = 0;

void handlerSignal(int sig) {
    stop_flag = 1;
}

void* calculate(void* arg) {
    data* cur_data = (data*)arg;
    long long i = cur_data->number;
    int step = cur_data->total_threads;

    double local_sum = 0.0;
    int count = 0;

    while (1) {
        local_sum += 1.0 / (i * 4.0 + 1.0);
        local_sum -= 1.0 / (i * 4.0 + 3.0);
        i += step;
        count++;

        if (count % INTERVAL == 0) {
            if(stop_flag == 1) {
                break;
            }
            count = 0;

        }
    }


    *(cur_data->sum) = local_sum;
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <num_threads>\n", argv[0]);
        exit(1);
    }

    int num_threads = atoi(argv[1]);

    if (signal(SIGINT, handlerSignal) == SIG_ERR) {
        perror("signal");
        exit(1);
    }

    data* data_array = (data*)malloc(num_threads * sizeof(data));
    pthread_t* threads_array = (pthread_t*)malloc(num_threads * sizeof(pthread_t));

    if (!data_array || !threads_array) {
        perror("malloc");
        exit(1);
    }

    for (int i = 0; i < num_threads; i++) {
        data_array[i].sum = malloc(sizeof(double));
        *(data_array[i].sum) = 0.0;
        data_array[i].number = i;
        data_array[i].total_threads = num_threads;

        if (pthread_create(&threads_array[i], NULL, calculate, &data_array[i]) != 0) {
            perror("pthread_create");
            exit(1);
        }
    }

    double pi = 0.0;
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads_array[i], NULL);
        pi += *(data_array[i].sum);
        free(data_array[i].sum);
    }

    pi *= 4.0;
    printf("pi done - %.10g \n", pi);

    free(data_array);
    free(threads_array);
    return 0;
}
