#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#define ITERATIONS 10

pthread_mutex_t mutex_S;
pthread_mutex_t mutex_P;
pthread_mutex_t mutex_C;

volatile int flag = 0;

void *child_thread(void *arg)
{
    int status;

    status = pthread_mutex_lock(&mutex_C);
    assert(status == 0);

    flag = 1;

    for (int i = 0; i < ITERATIONS; i++)
    {
        status = pthread_mutex_lock(&mutex_P);
        assert(status == 0);

        printf("Child thread: Line %d\n", i + 1);

        status = pthread_mutex_unlock(&mutex_C);
        assert(status == 0);

        status = pthread_mutex_lock(&mutex_S);
        assert(status == 0);

        status = pthread_mutex_unlock(&mutex_P);
        assert(status == 0);

        status = pthread_mutex_lock(&mutex_C);
        assert(status == 0);

        status = pthread_mutex_unlock(&mutex_S);
        assert(status == 0);
    }

    status = pthread_mutex_unlock(&mutex_C);
    assert(status == 0);

    return NULL;
}

int main()
{
    pthread_t thread;
    int status;
    pthread_mutexattr_t attr;

    status = pthread_mutexattr_init(&attr);
    if (status != 0)
    {
        fprintf(stderr, "ERROR initializing mutex attributes: %s\n", strerror(status));
        exit(EXIT_FAILURE);
    }

    status = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    if (status != 0)
    {
        fprintf(stderr, "ERROR setting mutex type: %s\n", strerror(status));
        exit(EXIT_FAILURE);
    }

    status = pthread_mutex_init(&mutex_S, &attr);
    if (status != 0)
    {
        fprintf(stderr, "ERROR initializing synchronization mutex: %s\n", strerror(status));
        exit(EXIT_FAILURE);
    }

    status = pthread_mutex_init(&mutex_P, &attr);
    if (status != 0)
    {
        fprintf(stderr, "ERROR initializing parent mutex: %s\n", strerror(status));
        exit(EXIT_FAILURE);
    }

    status = pthread_mutex_init(&mutex_C, &attr);
    if (status != 0)
    {
        fprintf(stderr, "ERROR initializing child mutex: %s\n", strerror(status));
        exit(EXIT_FAILURE);
    }

    status = pthread_mutexattr_destroy(&attr);
    if (status != 0)
    {
        fprintf(stderr, "ERROR destroying mutex attributes: %s\n", strerror(status));
        exit(EXIT_FAILURE);
    }

    status = pthread_mutex_lock(&mutex_P);
    assert(status == 0);

    status = pthread_create(&thread, NULL, child_thread, NULL);
    if (status != 0)
    {
        fprintf(stderr, "ERROR creating thread: %s\n", strerror(status));
        exit(EXIT_FAILURE);
    }

    while (flag != 1)
    {
        sleep(1);
    }

    for (int i = 0; i < ITERATIONS; i++)
    {
        printf("Parent thread: Line %d\n", i + 1);

        status = pthread_mutex_lock(&mutex_S);
        assert(status == 0);

        status = pthread_mutex_unlock(&mutex_P);
        assert(status == 0);

        status = pthread_mutex_lock(&mutex_C);
        assert(status == 0);

        status = pthread_mutex_unlock(&mutex_S);
        assert(status == 0);

        status = pthread_mutex_lock(&mutex_P);
        assert(status == 0);

        status = pthread_mutex_unlock(&mutex_C);
        assert(status == 0);
    }

    status = pthread_mutex_unlock(&mutex_P);
    assert(status == 0);

    status = pthread_join(thread, NULL);
    if (status != 0)
    {
        fprintf(stderr, "ERROR joining thread: %s\n", strerror(status));
        exit(EXIT_FAILURE);
    }

    status = pthread_mutex_destroy(&mutex_S);
    if (status != 0)
    {
        fprintf(stderr, "ERROR destroying synchronization mutex: %s\n", strerror(status));
        exit(EXIT_FAILURE);
    }

    status = pthread_mutex_destroy(&mutex_P);
    if (status != 0)
    {
        fprintf(stderr, "ERROR destroying parent mutex: %s\n", strerror(status));
        exit(EXIT_FAILURE);
    }

    status = pthread_mutex_destroy(&mutex_C);
    if (status != 0)
    {
        fprintf(stderr, "ERROR destroying child mutex: %s\n", strerror(status));
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}