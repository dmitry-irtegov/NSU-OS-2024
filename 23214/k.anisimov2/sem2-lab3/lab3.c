#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

void *thread_body(void *arg)
{
    char **data = (char **)arg;
    int index = 0;
    while (data[index] != NULL)
    {
        printf("%s\n", data[index++]);
    }
    pthread_exit(NULL);
}

int main()
{
    pthread_t threads[4];
    int code;

    char *line1[] = {"line 1.1", "line 1.2", "line 1.3", NULL};
    char *line2[] = {"line 2.1", NULL};
    char *line3[] = {"line 3.1", "line 3.2", "line 3.3", "line 3.4", "line 3.5", NULL};
    char *line4[] = {"line 4.1", "line 4.2", NULL};

    char **threadData[] = {line1, line2, line3, line4};

    for (int i = 0; i < 4; ++i)
    {
        if ((code = pthread_create(&threads[i], NULL, thread_body, (void *)threadData[i])) != 0)
        {
            fprintf(stderr, "ERROR creating thread: %s\n", strerror(code));
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < 4; ++i)
    {
        if ((code = pthread_join(threads[i], NULL)) != 0)
        {
            fprintf(stderr, "ERROR joining thread: %s\n", strerror(code));
            exit(EXIT_FAILURE);
        }
    }

    exit(EXIT_SUCCESS);
}