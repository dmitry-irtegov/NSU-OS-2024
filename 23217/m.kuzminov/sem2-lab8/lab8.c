#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct data {
    int left;
    int right;
    double* sum;
} data;

void* calculate(void* arg) {
    data* cur_data = (data*)arg;
    for (int i = cur_data->left; i < cur_data->right; i++) {
        *cur_data->sum += 1.0/(i*4.0 + 1.0);
        *cur_data->sum -= 1.0/(i*4.0 + 3.0);
    }
    return cur_data->sum;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        perror("You need number of threads");
        exit(1);
    }
    int steps = 200000000;
    int num_threads = atoi(argv[1]);

    data* data_array = (data*)malloc(num_threads * (sizeof(data)));
    pthread_t* threads_array = (pthread_t*)malloc(num_threads * (sizeof(pthread_t)));

    int per_thread = steps / num_threads;

    int balance = steps - per_thread * num_threads; 
    int left = 0;
    for (int i = 0; i < num_threads; i++) {
        data_array[i].left = left;
        data_array[i].right = (i + 1) * per_thread;
        data_array[i].sum = (double*)malloc(sizeof(double));
        *data_array[i].sum = 0;
        if(balance > 0) {
            data_array[i].right++;
            balance--;
        }
        left = data_array[i].right;

        if(pthread_create(&threads_array[i], NULL, calculate, (void*)&data_array[i])) {
            perror("Error creating thread");
            exit(1);
        }
    }

    double pi = 0;
    double* intermediate_result;

    for (int i = 0; i < num_threads; i++) {
        if(pthread_join(threads_array[i], (void**)&intermediate_result) != 0) {
            perror("Error joining thread");
            exit(1);
        }
        pi += *intermediate_result;
        free(intermediate_result);
    }
    pi *= 4;
    printf("pi done - %.10g \n", pi);
    free(data_array);
    free(threads_array);

    return 0;
}
