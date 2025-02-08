#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define num_steps 200000000

void* calc_series(void* args) {
    int* int_args = (int*) args;
    int start = int_args[0];
    int end = int_args[1];
    double* res = malloc(sizeof(double));

    if (res == NULL) {
        pthread_exit(NULL);
    }

    *res = 0;
    for (int i = start; i < end; i++) {    
        *res += 1.0/(i*4.0 + 1.0);
        *res -= 1.0/(i*4.0 + 3.0);
    }

    pthread_exit((void*) res);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Programm needs count of threads as a second arg\n");
        exit(EXIT_FAILURE);
    }

    int count_threads = atoi(argv[1]);
    if (count_threads < 1) {
        fprintf(stderr, "incorrect count of threads\n");
        exit(EXIT_FAILURE);
    }

    int step = num_steps / count_threads;
    
    pthread_t* threads = malloc(sizeof(pthread_t)*count_threads);
    int** args = malloc(sizeof(int*)*count_threads);
    if (args == NULL) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }

    int code;
    for (int i = 0; i < count_threads; i++) {
        args[i] = malloc(sizeof(int)*2);
        if (args[i] == NULL) {
            perror("malloc failed");
            exit(EXIT_FAILURE);
        }
        args[i][0] = step * i;
        if (i == (count_threads - 1)) {
            args[i][1] = num_steps;
        } else {
            args[i][1] = step * (i+1);
        }
        
        if ((code = pthread_create(&threads[i], NULL, &calc_series, (void*)args[i])) != 0) {
            fprintf(stderr, "pthread_create error: %d \n", code);
            exit(EXIT_FAILURE);
        }
    }

    double pi = 0;
    void* ret;
    for (int i = 0; i < count_threads; i++) {
        if ((code = pthread_join(threads[i], &ret)) != 0) {
            fprintf(stderr, "pthread_join error: &d\n", code);
            exit(EXIT_FAILURE);
        }

        if (ret == NULL) {
            fprintf(stderr, "An error occurred during the calculation of pi\n");
            exit(EXIT_FAILURE);
        }

        pi += *(double*)ret;
    }



    pi *= 4;
    printf("pi done - %.15g \n", pi);    
    
    exit(EXIT_SUCCESS);
}

