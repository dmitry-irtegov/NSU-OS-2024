#include<pthread.h>
#include<stdio.h>
#include<unistd.h>
#include<assert.h>

pthread_mutex_t mutex1;
pthread_mutex_t mutex2;
pthread_mutex_t mutex3;


volatile int initFlag = 0;

void* printer(void* unused)
{
    int opStatus;

    opStatus = pthread_mutex_lock(&mutex2);
    assert(opStatus==0);
    initFlag = 1;

    for(int i = 0; i<9; i++)
    {
        opStatus = pthread_mutex_lock(&mutex3);
        assert(opStatus==0);
        opStatus = pthread_mutex_unlock(&mutex2);
        assert(opStatus==0);
        opStatus = pthread_mutex_lock(&mutex1);
        assert(opStatus==0);
        printf("Child thread\n");
        opStatus = pthread_mutex_unlock(&mutex3); 
        assert(opStatus==0);
        opStatus = pthread_mutex_lock(&mutex2);
        assert(opStatus==0);
        opStatus = pthread_mutex_unlock(&mutex1);
        assert(opStatus==0);
    }

    
    opStatus = pthread_mutex_lock(&mutex3);
    assert(opStatus==0);
    opStatus = pthread_mutex_unlock(&mutex2);
    assert(opStatus==0);
    opStatus = pthread_mutex_lock(&mutex1);
    assert(opStatus==0);
    printf("Child thread\n");
    opStatus = pthread_mutex_unlock(&mutex3);
    assert(opStatus==0);
    opStatus = pthread_mutex_unlock(&mutex1);
    assert(opStatus==0);

    return NULL;
}


int main()
{ 
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);

    pthread_mutex_init(&mutex1, &attr); 
    pthread_mutex_init(&mutex2, &attr);
    pthread_mutex_init(&mutex3, &attr);
    pthread_mutexattr_destroy(&attr);

    int opStatus;
    
    
    opStatus = pthread_mutex_lock(&mutex3);
    assert(opStatus==0);
    opStatus = pthread_mutex_lock(&mutex1);
    assert(opStatus==0);

    pthread_t childThread;
    pthread_create(&childThread, NULL, printer, NULL);

    while(initFlag==0)
    {
        sched_yield();
    }


    for(int i = 0; i<9; i++)
    {
        printf("Parent thread\n");
        opStatus = pthread_mutex_unlock(&mutex3); 
        assert(opStatus==0);
        opStatus = pthread_mutex_lock(&mutex2);
        assert(opStatus==0);
        opStatus = pthread_mutex_unlock(&mutex1);
        assert(opStatus==0);
        opStatus = pthread_mutex_lock(&mutex3);
        assert(opStatus==0);
        opStatus = pthread_mutex_unlock(&mutex2);
        assert(opStatus==0);
        opStatus = pthread_mutex_lock(&mutex1);
        assert(opStatus==0);
    }
    printf("Parent thread\n");
    opStatus = pthread_mutex_unlock(&mutex3);
    assert(opStatus==0);
    opStatus = pthread_mutex_unlock(&mutex1);
    assert(opStatus==0);

    pthread_join(childThread, NULL);

    pthread_mutex_destroy(&mutex1);
    pthread_mutex_destroy(&mutex2);
    pthread_mutex_destroy(&mutex3);
    return 0;
}
