#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct {
    char** strs;
} Array;

void* pthreadFunc(void *data) {
    Array* strs = (Array*) data;
    for (int i = 0; i < 4; i++) {
        printf("%s\n", strs->strs[i]);
    }
    pthread_exit(EXIT_SUCCESS);
}

int main() {
    pthread_t *threads = NULL;
    pthread_attr_t attr;
    int errID = 0;

    char* arr1[] = {"1 1", "1 2", "1 3", "1 4"};
    char* arr2[] = {"2 1", "2 2", "2 3", "2 4"};
    char* arr3[] = {"3 1", "3 2", "3 3", "3 4"};
    char* arr4[] = {"4 1", "4 2", "4 3", "4 4"};

    Array strs[4] = {{arr1}, {arr2}, {arr3}, {arr4}};

    if ((errID = pthread_attr_init(&attr)) != 0) {
        fprintf(stderr, "ERROR: failed in pthread_attr_init. Program ended with code %d\n", errID);
        exit(EXIT_FAILURE);
    }

    threads = malloc(sizeof(pthread_t) * 4);
    if (threads == NULL) {
        fprintf(stderr, "ERROR: failed to allocate memory. Program ended with code %d\n", errID);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < 4; i++) {
        if ((errID = pthread_create(&(threads[i]), &attr, pthreadFunc, (void*)&strs[i])) != 0) {
            fprintf(stderr, "ERROR: failed to create thread. Program ended with code %d\n", errID);
            exit(EXIT_FAILURE);
        }
		if ((errID = pthread_join(threads[i], NULL)) != 0) {
            fprintf(stderr, "ERROR: failed to join a thread. Program ended with code %d\n", errID);
            exit(EXIT_FAILURE);
        }
    }

    if ((errID = pthread_attr_destroy(&attr)) != 0) {
        fprintf(stderr, "ERROR: failed to destroy the attr. Program ended with code %d\n", errID);
        exit(EXIT_FAILURE);
    }

    free(threads);
    pthread_exit(EXIT_SUCCESS);
}
