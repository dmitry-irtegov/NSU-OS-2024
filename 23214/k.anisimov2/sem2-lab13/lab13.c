#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <assert.h>

#define ITERATIONS 10

pthread_mutex_t mutex;
pthread_cond_t cond;
int turn = 0; // 0 - parent, 1 - child

void *child_thread(void *arg)
{
    int status;

    for (int i = 0; i < ITERATIONS; ++i)
    {
        status = pthread_mutex_lock(&mutex);
        assert(status == 0);

        while (turn != 1)
        {
            status = pthread_cond_wait(&cond, &mutex);
            if (status != 0)
            {
                fprintf(stderr, "ERROR waiting on condition in child thread: %s\n", strerror(status));
                exit(EXIT_FAILURE);
            }
        }

        printf("Child thread: Line %d\n", i + 1);
        turn = 0;

        status = pthread_cond_signal(&cond);
        if (status != 0)
        {
            fprintf(stderr, "ERROR signaling condition in child thread: %s\n", strerror(status));
            exit(EXIT_FAILURE);
        }

        status = pthread_mutex_unlock(&mutex);
        assert(status == 0);
    }

    return NULL;
}

int main()
{
    pthread_t thread;
    int status;

    status = pthread_mutex_init(&mutex, NULL);
    if (status != 0)
    {
        fprintf(stderr, "ERROR initializing mutex: %s\n", strerror(status));
        exit(EXIT_FAILURE);
    }

    status = pthread_cond_init(&cond, NULL);
    if (status != 0)
    {
        fprintf(stderr, "ERROR initializing condition variable: %s\n", strerror(status));
        exit(EXIT_FAILURE);
    }

    status = pthread_create(&thread, NULL, child_thread, NULL);
    if (status != 0)
    {
        fprintf(stderr, "ERROR creating thread: %s\n", strerror(status));
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < ITERATIONS; ++i)
    {
        status = pthread_mutex_lock(&mutex);
        assert(status == 0);

        while (turn != 0)
        {
            status = pthread_cond_wait(&cond, &mutex);
            if (status != 0)
            {
                fprintf(stderr, "ERROR waiting on condition in parent thread: %s\n", strerror(status));
                exit(EXIT_FAILURE);
            }
        }

        printf("Parent thread: Line %d\n", i + 1);
        turn = 1;

        status = pthread_cond_signal(&cond);
        if (status != 0)
        {
            fprintf(stderr, "ERROR signaling condition in parent thread: %s\n", strerror(status));
            exit(EXIT_FAILURE);
        }

        status = pthread_mutex_unlock(&mutex);
        assert(status == 0);
    }

    status = pthread_join(thread, NULL);
    if (status != 0)
    {
        fprintf(stderr, "ERROR joining thread: %s\n", strerror(status));
        exit(EXIT_FAILURE);
    }

    status = pthread_mutex_destroy(&mutex);
    if (status != 0)
    {
        fprintf(stderr, "ERROR destroying mutex: %s\n", strerror(status));
        exit(EXIT_FAILURE);
    }

    status = pthread_cond_destroy(&cond);
    if (status != 0)
    {
        fprintf(stderr, "ERROR destroying condition variable: %s\n", strerror(status));
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
