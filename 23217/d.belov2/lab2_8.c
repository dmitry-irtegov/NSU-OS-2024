#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define num_steps 200000000

typedef struct
{
    int start_index;
    int step_for_index; // based on num threads
    double sum;
} thread_info;

void *find_pi(void *arg)
{
    thread_info *info = (thread_info *)arg;
    double sum = 0.0;
    for (int i = info->start_index; i < num_steps; i += info->step_for_index)
    {
        sum += 1.0 / (i * 4.0 + 1.0);
        sum -= 1.0 / (i * 4.0 + 3.0);
    }
    info->sum = sum;
    pthread_exit(NULL);
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("args count err\n");
        return 1;
    }

    int num_threads = atoi(argv[1]);
    if (num_threads <= 0)
    {
        printf("threads < 1.\n");
        return 1;
    }

    pthread_t threads[num_threads];
    thread_info thread_info[num_threads];
    double pi = 0.0;

    for (int i = 0; i < num_threads; i++)
    {
        thread_info[i].start_index = i;
        thread_info[i].step_for_index = num_threads;
        thread_info[i].sum = 0.0;
        if (pthread_create(&threads[i], NULL, find_pi, &thread_info[i]) != 0)
        {
            printf("err create thread: %d.\n", i);
            return 1;
        }
    }

    for (int i = 0; i < num_threads; i++)
    {
        if (pthread_join(threads[i], NULL) != 0)
        {
            printf("err join thread: %d.\n", i);
            return 1;
        }
        pi += thread_info[i].sum;
    }

    pi = pi * 4.0;
    printf("pi done - %.15g\n", pi);

    return 0;
}