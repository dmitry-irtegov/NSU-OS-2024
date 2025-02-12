#include <stdio.h>
#include <semaphore.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#define ITERATIONS 10

sem_t a; // buffer is empty
sem_t b; // buffer is full

void *child_thread(void *args)
{
    int status;

    for (int i = 0; i < ITERATIONS; i++)
    {
        status = sem_wait(&b);
        if (status != 0)
        {
            fprintf(stderr, "ERROR in child thread with sem_wait: %s\n", strerror(status));
            pthread_exit(NULL);
        }

        printf("Child thread: Line %d\n", i + 1);

        status = sem_post(&a);
        if (status != 0)
        {
            fprintf(stderr, "ERROR in child thread with sem_post: %s\n", strerror(status));
            pthread_exit(NULL);
        }
    }
    
    return NULL;
}

int main()
{
    pthread_t thread;
    int status;

    status = sem_init(&a, 0, 1);
    if (status != 0)
    {
        fprintf(stderr, "ERROR initializing semaphore a: %s\n", strerror(status));
        exit(EXIT_FAILURE);
    }

    status = sem_init(&b, 0, 0);
    if (status != 0)
    {
        fprintf(stderr, "ERROR initializing semaphore b: %s\n", strerror(status));
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
        status = sem_wait(&a);
        if (status != 0)
        {
            fprintf(stderr, "ERROR in parent thread with sem_wait: %s\n", strerror(status));
            exit(EXIT_FAILURE);
        }

        printf("Parent thread: Line %d\n", i + 1);

        status = sem_post(&b);
        if (status != 0)
        {
            fprintf(stderr, "ERROR in parent thread with sem_post: %s\n", strerror(status));
            exit(EXIT_FAILURE);
        }
    }

    status = pthread_join(thread, NULL);
    if (status != 0)
    {
        fprintf(stderr, "ERROR joining thread: %s\n", strerror(status));
        exit(EXIT_FAILURE);
    }

    status = sem_destroy(&a);
    if (status != 0)
    {
        fprintf(stderr, "ERROR destroying semaphore a: %s\n", strerror(status));
        exit(EXIT_FAILURE);
    }

    status = sem_destroy(&b);
    if (status != 0)
    {
        fprintf(stderr, "ERROR destroying semaphore b: %s\n", strerror(status));
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
