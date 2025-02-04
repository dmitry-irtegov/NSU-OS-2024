#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define num_steps 200000000

void* calc_series(void* args) {
    int start = ((int*)args)[0];
    int end = ((int*)args)[1];
    double* res = malloc(sizeof(double));
    *res = 0;
    printf("%d %d\n", start, end);
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
    int* args = malloc(sizeof(int)*2);

    for (int i = 0; i < count_threads; i++) {
        args[0] = step * i;
        if (i == (count_threads - 1)) {
            args[1] = num_steps;
        } else {
            args[1] = step * (i+1);
        }
        
        pthread_create(&threads[i], NULL, calc_series, (void*)args);
    }

    double pi = 0;
    void* ret;
    for (int i = 0; i < count_threads; i++) {
        pthread_join(threads[i], &ret);
        pi += *((double*)ret);
    }

    printf("pi done - %.15g \n", pi);    
    
    exit(EXIT_SUCCESS);
}

