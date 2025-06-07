#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>

sem_t parent, child;

void* thread_body(void* param) {
    for(int i = 0; i < 10; i++) {
        sem_wait(&child);
        printf("Hi, I'm a Child with number: %d\n", i);
        sem_post(&parent);
    }
        
  pthread_exit(NULL);
}

int main(int argc, char *argv[]) 
{
    pthread_t thread;
    sem_init(&parent, 0, 1);
    sem_init(&child, 0, 0);
    int value;
    
    if ((value = pthread_create(&thread, NULL, thread_body, NULL)) != 0 ) {
        perror("Error in pthread_creation.");
        exit(EXIT_FAILURE);
    }
    
    for(int i = 0; i < 10; i++) {
        sem_wait(&parent);
        printf("Hi, I'm a Parent with number: %d\n", i);
        sem_post(&child);
    }
        
    pthread_exit(NULL); 
}
