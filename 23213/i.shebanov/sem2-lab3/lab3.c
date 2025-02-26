#include<stdlib.h>
#include<stdio.h>
#include<pthread.h>
#include<string.h>


static char * s1[] = { "thread 1", "text of thread 1", "more thread 1", "thread 1 text", NULL };
static char * s2[] = { "thread 2", "text of thread 2", "thread 2 text", NULL };
static char * s3[] = { "thread 3", "text of thread 3", NULL };
static char * s4[] = { "thread 4", "text of thread 4", "more thread 4", "thread 4 text", NULL };

void * print(void * strings)
{
    char ** arr = (char**)strings;

    int i = 0;
    while(arr[i] != NULL)
    {
        printf("%s\n", arr[i]);
        i++;
    }

    return NULL;
}

int main()
{
    pthread_t threads[4];
    char ** strings[4] = {s1, s2, s3, s4};

    for(int i = 0; i < 4; i++)
    {
        int code;
        if((code = pthread_create(&threads[i], NULL, print, (void*)strings[i])) != 0)
        {
            fprintf(stderr, "%s while creating thread: %d\n", strerror(code), i);
        }
    }

    pthread_exit(NULL);
}
