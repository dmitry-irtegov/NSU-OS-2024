#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#define SIZE 100
#define MAX_LENGTH 10
#define SLEEP_TIME 5200

char array_of_strings[SIZE][MAX_LENGTH];

void *sleeping(void *index)
{
    intptr_t ind = (intptr_t)index;
    usleep(SLEEP_TIME * strlen(array_of_strings[ind]));
    printf("%s", array_of_strings[ind]);
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    int real_size = 0;
    for (int i = 0; i < SIZE; i++)
    {
        char *p = fgets(array_of_strings[i], MAX_LENGTH, stdin);
        if (p == NULL)
        {
            real_size = i;
            break;
        }
    }
    printf("\n----------------------\n");

    pthread_t thread;

    for (int i = 0; i < real_size; i++)
    {
        int code = pthread_create(&thread, NULL, sleeping, (void *)(intptr_t)i);
        if (code != 0)
        {
            fprintf(stderr, "Error in pthread_create: %s\n", strerror(code));
            exit(EXIT_FAILURE);
        }
    }
    pthread_exit(NULL);
}
