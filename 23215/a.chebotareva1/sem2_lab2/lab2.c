#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

void *thread_func(){
    for(int i = 1; i < 11; i++) {
        printf("Thread line num %d\n", i);
    }
    return NULL;
}

int main(){
    pthread_t thread;
    if (pthread_create(&thread, NULL, thread_func, NULL) != 0){
        printf("Error with thread creating");
        return EXIT_FAILURE;
    }
    pthread_join(thread, NULL);
    for(int i = 1; i < 11; i++) {
        printf("Parent line num %d\n", i);
    }
    pthread_exit(NULL);
}