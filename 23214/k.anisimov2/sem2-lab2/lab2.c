#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

void *thread_body(void *param)
{
    for (int i = 1; i < 110; ++i)
    {
        printf("Newly created thread: line %d\n", i);
    }
    return NULL;
}

int main()
{
    pthread_t thread;
    int code;

    if ((code = pthread_create(&thread, NULL, thread_body, NULL)) != 0)
    {
        fprintf(stderr, "ERROR creating thread: %s\n", strerror(code));
        exit(EXIT_FAILURE);
    }

    if ((code = pthread_join(thread, NULL)) != 0)
    {
        fprintf(stderr, "ERROR joining thread: %s\n", strerror(code));
        exit(EXIT_FAILURE);
    }

    for (int i = 1; i < 101; ++i)
    {
        printf("Parent thread: line %d\n", i);
    }

    pthread_exit(EXIT_SUCCESS);
}
