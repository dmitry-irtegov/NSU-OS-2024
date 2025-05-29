#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define ITERATIONS 100000000

typedef struct
{
    long start;
    long end;
} ThreadData;

void *child_thread(void *arg)
{
    ThreadData *data = (ThreadData *)arg;
    double *partial_sum = malloc(sizeof(double));

    if (partial_sum == NULL)
    {
        fprintf(stderr, "ERROR allocating memory for partial sum\n");
        pthread_exit(NULL);
    }

    *partial_sum = 0.0;

    for (long i = data->start; i < data->end; ++i)
    {
        *partial_sum += (i % 2 == 0 ? 1.0 : -1.0) / (2 * i + 1);
    }

    pthread_exit(partial_sum);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "ERROR: Has to be like this: %s {number of threads}\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int threads_num = atoi(argv[1]);
    if (threads_num <= 0)
    {
        fprintf(stderr, "ERROR not enough threads\n");
        exit(EXIT_FAILURE);
    }

    pthread_t threads[threads_num];
    ThreadData thread_data[threads_num];

    long thread_steps = ITERATIONS / threads_num;
    long remainder = ITERATIONS % threads_num;

    int status;

    long cur_pos = 0;

    for (int i = 0; i < threads_num; ++i)
    {
        long steps = thread_steps + (i < remainder ? 1 : 0);
        thread_data[i].start = cur_pos;
        thread_data[i].end = cur_pos + steps;
        cur_pos += steps;

        status = pthread_create(&threads[i], NULL, child_thread, &thread_data[i]);
        if (status != 0)
        {
            fprintf(stderr, "ERROR creating thread: %s\n", strerror(status));
            exit(EXIT_FAILURE);
        }
    }

    double pi = 0.0;
    for (int i = 0; i < threads_num; ++i)
    {
        void *res;
        status = pthread_join(threads[i], &res);
        if (status != 0)
        {
            fprintf(stderr, "ERROR joining thread: %s\n", strerror(status));
            exit(EXIT_FAILURE);
        }

        double *partial_sum = (double *)res;
        pi += *partial_sum;

        free(partial_sum);
    }
    pi *= 4.0;
    printf("PI = %.15f\n", pi);
    exit(EXIT_SUCCESS);
}