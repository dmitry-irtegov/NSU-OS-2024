#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

char* s1[] = { "thread 1 text 1", "thread 1 text 2", "thread 1 text 3", "thread 1 text 4", NULL };
char* s2[] = { "thread 2 text 1", "thread 2 text 2", "thread 2 text 3", NULL };
char* s3[] = { "thread 3 text 1", "thread 3 text 2", NULL };
char* s4[] = { "thread 4 text 1", "thread 4 text 2", "thread 4 text 3", "thread 4 text 4", NULL };

void* thread_body(void* param) {
    int i = 0;
    for (char** strings = (char**)param; strings[i] != NULL; i++)
    {
        printf("%s\n", strings[i]);
    }
    return NULL;
}

int main (int argc, char* argv[]) {
    char ** strings[4] = { s1, s2, s3, s4 };
    pthread_t threads[4];
    int code;

    for (int i = 0; i < 4; i++) {
        if ((code = pthread_create(&threads[i], NULL, thread_body, (void*)strings[i])) != 0) {
            fprintf(stderr, "%s: creating thread: %s\n", argv[0], strerror(code));
            exit(1);
        }
    }

    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }
    return 0;
}
