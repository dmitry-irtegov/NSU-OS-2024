#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

void err_handler(char* msg, int errID){
	fprintf(stderr, "%s %s\n", msg, strerror(errID));
}

void* pthreadFunc(void *data){
    char** strs = (char**) data;
    for (int i = 0; i < 4; i++){
        printf("%s\n", strs[i]);
    }
    pthread_exit(0);
}

int main(){
    pthread_t *threads = NULL;
    pthread_attr_t attr;
    int errID = 0;

    char* str1[] = {"1 1", "1 2", "1 3", "1 4"};
    char* str2[] = {"2 1", "2 2", "2 3", "2 4"};
    char* str3[] = {"3 1", "3 2", "3 3", "3 4"};
    char* str4[] = {"4 1", "4 2", "4 3", "4 4"};

    char** strs[] = {str1, str2, str3, str4};

    if ((errID = pthread_attr_init(&attr)) != 0){
        err_handler("ERROR: failed to init attr. Program ended with code", errID);
        exit(EXIT_FAILURE);
    }

    threads = malloc(sizeof(pthread_t) * 4);
    if (threads == NULL) {
        err_handler("ERROR: failed to allocate memory. Program ended with code", errID);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < 4; i++) {
        if ((errID = pthread_create(&(threads[i]), &attr, pthreadFunc, (void*) strs[i])) != 0){
            err_handler("ERROR: failed to create thread. Program ended with code", errID);
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < 4; i++){
        if ((errID = pthread_join(threads[i], NULL)) != 0){
            err_handler("ERROR: failed to join a thread. Program ended with code", errID);
            exit(EXIT_FAILURE);
        }
    }

    if ((errID = pthread_attr_destroy(&attr)) != 0){
        err_handler("ERROR: failed to destroy the attr. Program ended with code", errID);
        exit(EXIT_FAILURE);
    }

    free(threads);
    pthread_exit(0);
}
