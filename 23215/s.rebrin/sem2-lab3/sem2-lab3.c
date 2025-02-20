#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>


void* print_strings(void* arg) {
    char** data = (char**)arg;
    for (int i = 0; data[i]; i++) {
        printf("%s\n",data[i]);
    }
    pthread_exit(NULL);
}

int main() {

    char* strings1[] = { "one", "uno", "odin", "1", NULL };
    char* strings2[] = { "two", "dos", "dwa", "2", NULL };
    char* strings3[] = { "three", "tres", "tri", "3", NULL };
    char* strings4[] = { "four", "quadro", "chetyre", "4", NULL };

    char** thread_strings[] = { strings1, strings2, strings3, strings4};

    pthread_t threads[4];

    for (int i = 0; i < 4; i++) {

        int rc = pthread_create(&threads[i], NULL, print_strings, (void*)thread_strings[i]);
        if (rc) {
            fprintf(stderr, "Error: Unable to create thread %d, return code %d\n", i + 1, rc);
            exit(1);
        }
    }

    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}