#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define ITERATIONS 10

#define SHARED_MUTEX 0
#define PARENT_MUTEX 1
#define CHILD_MUTEX  2

pthread_mutex_t mutex[3];

void check_error(int err_code)
{
    if (err_code != 0)
    {
        fprintf(stderr, "%d\n", err_code);
        exit(EXIT_FAILURE);
    }
}

void *child_function(void *arg)
{
    check_error(pthread_mutex_lock(&mutex[CHILD_MUTEX]));

    for (int i = 0; i < ITERATIONS; i++)
    {
        check_error(pthread_mutex_lock(&mutex[PARENT_MUTEX]));

        printf("child  :  %d\n", i + 1);

        check_error(pthread_mutex_unlock(&mutex[CHILD_MUTEX]));
        check_error(pthread_mutex_lock(&mutex[SHARED_MUTEX]));
        check_error(pthread_mutex_unlock(&mutex[PARENT_MUTEX]));
        check_error(pthread_mutex_lock(&mutex[CHILD_MUTEX]));
        check_error(pthread_mutex_unlock(&mutex[SHARED_MUTEX]));
    }

    check_error(pthread_mutex_unlock(&mutex[CHILD_MUTEX]));

    return NULL;
}

int main()
{
    pthread_t child_thread;
    pthread_mutexattr_t attr;
    int err;

    err = pthread_mutexattr_init(&attr);
    check_error(err);

    err = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    check_error(err);

    for (int i = 0; i < 3; i++)
    {
        err = pthread_mutex_init(&mutex[i], &attr);
        check_error(err);
    }

    check_error(pthread_mutex_lock(&mutex[PARENT_MUTEX]));

    err = pthread_create(&child_thread, NULL, child_function, NULL);
    check_error(err);

    for (int i = 0; i < ITERATIONS; i++)
    {
        printf("parent :  %d\n", i + 1);

        check_error(pthread_mutex_lock(&mutex[SHARED_MUTEX]));
        check_error(pthread_mutex_unlock(&mutex[PARENT_MUTEX]));
        check_error(pthread_mutex_lock(&mutex[CHILD_MUTEX]));
        check_error(pthread_mutex_unlock(&mutex[SHARED_MUTEX]));
        check_error(pthread_mutex_lock(&mutex[PARENT_MUTEX]));
        check_error(pthread_mutex_unlock(&mutex[CHILD_MUTEX]));
    }

    check_error(pthread_mutex_unlock(&mutex[PARENT_MUTEX]));

    check_error(pthread_join(child_thread, NULL));

    for (int i = 0; i < 3; i++)
    {
        err = pthread_mutex_destroy(&mutex[i]);
        check_error(err);
    }

    check_error(pthread_mutexattr_destroy(&attr));

    return 0;
}
