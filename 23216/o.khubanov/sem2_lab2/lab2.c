#include <stdio.h>    
#include <stdlib.h>   
#include <pthread.h>  
#include <errno.h>  

#define handleerror(en, msg) do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

void* print_lines() {
    for (int i = 0; i < 10; i++) {
        printf("%d\n", i);
        fflush(stdout); // Принудительный сброс буфера
    }
    return NULL;
}

int main() {
    pthread_t thread;
    
    int val1 = pthread_create(&thread, NULL, &print_lines, NULL); 
    if (val1 != 0){ 
        handleerror(val1, "pthread_create");
        }
        
    // Ожидание завершения дочернего потока
    int val2=pthread_join(thread, NULL);
    if (val2 != 0){
    	handleerror(val2, "pthread_create");
    }
    
    // Выполнение кода в главном потоке после завершения дочернего
    print_lines();
    
    return 0;
}

