#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include<string.h>
#include<semaphore.h>

sem_t s1, s2;

void * printer(void * unused)
{
    for(int i = 0; i < 10; i++)
    {
        sem_wait(&s2);
        printf("Child thread\n");
        sem_post(&s1);
    }
    return NULL;
}

int main()
{
    sem_init(&s1, 0, 1);
    sem_init(&s2, 0, 0);

    pthread_t thread;
    int err = pthread_create(&thread, NULL, printer, NULL);
    if(err != 0)
    {
        fprintf(stderr, "Pthread creation failed: %s\n", strerror(err));
        sem_destroy(&s1);
        sem_destroy(&s2);
        return 1;
    }

    for(int i = 0; i<10; i++)
    {
        sem_wait(&s1);
        printf("Parent thread\n");
        sem_post(&s2);
    }

    pthread_join(thread, NULL);

    sem_destroy(&s1);
    sem_destroy(&s2);
    return 0;
}
