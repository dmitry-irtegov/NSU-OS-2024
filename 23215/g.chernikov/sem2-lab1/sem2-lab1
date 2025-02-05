#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

void* childPrinting() {

    for(int i=0; i<10; i++){
        printf("Child\n");
    }

	return NULL;
}

int main() {
    pthread_t thread;

    pthread_create(&thread, NULL, childPrinting, NULL);
    
    for(int i=0; i<10; i++){
        printf("Parent\n");
    }
    
    pthread_exit(NULL);
}