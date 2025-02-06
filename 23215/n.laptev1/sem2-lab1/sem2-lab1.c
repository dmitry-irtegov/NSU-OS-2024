#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

void* thread_body(void* param) {
    for(int i = 0; i < 10; i++) 
        printf("Hi, I'm a Child\n");
  pthread_exit(NULL);
}

int main(int argc, char *argv[]) 
{
    pthread_t thread;
    int ID;
    
    if ((ID = pthread_create(&thread, NULL, thread_body, NULL)) != 0 ) 
    {
        perror("Error in pthread_creation.");
        exit(EXIT_FAILURE);
    }
    
    for(int i = 0; i < 10; i++) 
    {
        printf("Hi, I'm a Parent.\n");
    }
        
    pthread_exit(NULL); 
}
