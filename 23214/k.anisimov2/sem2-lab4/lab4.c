#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

void *thread_body(void *arg)
{
    while (1)
    {
        printf("Child thread is running.\n");
        sleep(1);
    }
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

    sleep(2);
    if ((code = pthread_cancel(thread)) != 0)
    {
        fprintf(stderr, "ERROR canceling thread: %s\n", strerror(code));
        exit(EXIT_FAILURE);
    }
    if ((code = pthread_join(thread, NULL)) != 0)
    {
        fprintf(stderr, "ERROR joining thread: %s\n", strerror(code));
        exit(EXIT_FAILURE);
    }

    printf("The child thread was successfully annihilated, now exit.\n");

    exit(EXIT_SUCCESS);
}
