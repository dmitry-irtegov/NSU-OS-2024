#include <stdio.h>
#include <stdlib.h>
#include <pthread.h> 

void* childPthread(void* arg) {
    for(int i = 0; i < 10; i++) {
        printf("Child thread: line %d\n", i + 1);
    }
    return NULL;
}

int main() {
    pthread_t thread;

    int newPthread = pthread_create(&thread, NULL, childPthread, NULL);

    if(newPthread != 0) {
        fprintf(stderr, "Error creating thread\n");
        return 1;
    }

    pthread_join(thread, NULL);

    for(int i = 0; i < 10; i++) {
        printf("Parent thread: line %d\n", i + 1);
    }

    pthread_exit(NULL);

}