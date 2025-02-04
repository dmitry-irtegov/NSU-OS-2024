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
    pthread_attr_t attr;

    // Инициализация атрибутов потока
    int val = pthread_attr_init(&attr); 
    if (val != 0){ 
        handleerror(val, "pthread_attr_init");\
        } 
        

    int val1 = pthread_create(&thread, &attr, &print_lines, NULL); 
    if (val1 != 0){ 
    	pthread_attr_destroy(&attr); // Освобождаем атрибуты перед выходом
        handleerror(val1, "pthread_create");
        }
        
        
    // Очистка атрибутов после использования
    int val2 = pthread_attr_destroy(&attr);
    if (val2 != 0){
        handleerror(val2, "pthread_attr_destroy");
        }
        
    // Выполнение кода в главном потоке
    print_lines(NULL);

    // Завершаем главный поток, позволяя дочернему завершиться
    pthread_exit(NULL);
}

