#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#define NUM_STEPS_PER_ITERATION 1000000

unsigned long myindex = 0;
unsigned char stop = 0;

pthread_mutex_t index_mutex;
pthread_mutex_t stop_mutex;

void* pi_calculation(void *param) {
    double *partial_sum = (double *)malloc(sizeof(double));
    *partial_sum = 0;

    unsigned long curr_index;
    unsigned char cont = 1;

    while (cont) {

        pthread_mutex_lock(&index_mutex);
        curr_index = myindex++;
        pthread_mutex_unlock(&index_mutex);

        for (unsigned long i = curr_index * NUM_STEPS_PER_ITERATION; i < (curr_index + 1) * NUM_STEPS_PER_ITERATION; i++) {
            *partial_sum += 1.0/(i*4.0 + 1.0);
            *partial_sum -= 1.0/(i*4.0 + 3.0);
        }

        // printf("index = %ld; partsum = %0.15g\n", index, *partial_sum);

        pthread_mutex_lock(&stop_mutex);
        if (stop == 1) {
            cont = 0;
        }
        pthread_mutex_unlock(&stop_mutex);
    }
    
    pthread_exit((void *)partial_sum);
}

int main(int argc, char *argv[]) {

    if (argc < 2) {
        fprintf(stderr, "a gde number of threads?\n");
        exit(EXIT_FAILURE);
    }

    int number_of_threads = atoi(argv[1]);

    if (number_of_threads < 1) {
        fprintf(stderr, "bad number of threads\n");
        exit(EXIT_FAILURE);
    }

    pthread_mutex_init(&index_mutex, NULL);
    pthread_mutex_init(&stop_mutex, NULL);

    sigset_t set;
    int sig;

    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    pthread_sigmask(SIG_BLOCK, &set, NULL);

    pthread_t *threads = (pthread_t *)malloc(sizeof(pthread_t) * number_of_threads);

    for (int i = 0; i < number_of_threads; i++) {
        if (pthread_create(&threads[i], NULL, pi_calculation, NULL) != 0) {
            fprintf(stderr, "can't create thread\n");
            exit(EXIT_FAILURE);
        }
    }

    sigwait(&set, &sig);

    pthread_mutex_lock(&stop_mutex);
    stop = 1;
    pthread_mutex_unlock(&stop_mutex);

    double pi = 0;
    double *part_sum;

    for (int i = 0; i < number_of_threads; i++) {
        pthread_join(threads[i], (void **)&part_sum);
        // printf("\n%d", i);
        pi += *(double *)part_sum;
        free(part_sum);
    }

    pi *= 4.0;
    printf("\npi done - %.30g \n", pi); 

    free(threads);
    pthread_mutex_destroy(&index_mutex);
    pthread_mutex_destroy(&stop_mutex);

    exit(EXIT_SUCCESS);
}
