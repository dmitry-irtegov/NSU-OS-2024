#include <stdio.h> 
#include <stdlib.h> 
#include <pthread.h> 
#include <errno.h> 
 
#define handle_error_en(en, msg) do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0) 
 
void * print_lines(void* arg) { 
    for (int i = 0; i < 10; i++) { 
        printf("Thread: number %d\n", i); 
    } 
    pthread_exit(0); 
} 
 
int main() { 
    pthread_t thread; 
    pthread_attr_t attr;
    int result;
 
 
    result = pthread_attr_init(&attr); 
    if (result != 0) 
        handle_error_en(result, "pthread_attr_init"); 
     
    result = pthread_create(&thread, &attr, &print_lines, NULL); 
    if (result != 0) 
        handle_error_en(result, "pthread_create");

    result = pthread_attr_destroy(&attr);
    if (result != 0)
        handle_error_en(result, "pthread_attr_destroy");
        
    result = pthread_join(thread, NULL); 
    if (result != 0) 
           handle_error_en(result, "pthread_join"); 


    for (int i = 0; i < 10; i++) { 
        printf("Parent thread: number %d\n", i); 
    } 
 

    exit(EXIT_SUCCESS);
}