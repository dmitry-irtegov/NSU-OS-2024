#include <stdio.h> 
#include <stdlib.h> 
#include <pthread.h> 
#include <errno.h> 
 
#define handle_error_en(en, msg) do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0) 
 
void print_all_lines(char* string){
    for (int i = 0; i < 10; i++) { 
        printf("%s: number %d\n", string, i); 
    }
}
 
void * print_lines(void* arg) { 
    print_all_lines("Thread");
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
        

    print_all_lines("Parent thread");

    pthread_exit(0);
}