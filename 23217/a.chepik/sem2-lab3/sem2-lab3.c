#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define minn(X, Y) (((X) < (Y)) ? (X) : (Y))
#define THREAD_COUNT 4

char* arr[] = { "thread 1", "text of thread 1", "thread 1 text", "more thread 1",
    "thread 2", "text of thread 2", "thread 2 text", "more thread 2",
    "thread 3", "text of thread 3", "thread 3 text",
    "thread 4", "text of thread 4", "thread 4 text" };

int str_count = sizeof(arr) / sizeof(char*);

void* thread_body(void* param) {
    int indx = *((int*)param);
    
    int int_part = str_count / THREAD_COUNT;
    int remainder = str_count % THREAD_COUNT;
    int count = int_part + (indx < remainder ? 1 : 0);
    
    char** arr_begin = arr + int_part * indx + minn(remainder, indx);

    for (int i = 0; i < count; i++) {
        printf("%s\n", arr_begin[i]);
    }

    return NULL;
}

int main() {
    int pc;
    pthread_t threads[THREAD_COUNT];

    int* indx = (int*)malloc(THREAD_COUNT * sizeof(int));

    for (int i = 0; i < THREAD_COUNT; i++) {
        indx[i] = i;

        pc = pthread_create(&threads[i], NULL, thread_body, &indx[i]);

        if (pc != 0) {
            printf("Error in pthread_create().\n");
            exit(-1);
        }
    }

    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }

    free(indx);

    exit(0);
}
