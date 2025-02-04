#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void* print_lines() {
    for (int i = 0; i < 10; i++) {
        printf("%d\n", i);
    }
    return NULL;
}

int main() {
    pthread_t thread;
    
    // Создание нового потока
    int val=pthread_create(&thread, NULL, print_lines, NULL);
    if (val != 0) {
        fprintf(stderr, "Ошибка при создании потока: %d\n", val);
        return EXIT_FAILURE;
    }
    
    // Ожидание завершения дочернего потока
    int val1=pthread_join(thread, NULL);
    if (val1 != 0){
    	fprintf(stderr, "Ошибка при ghbcjtlbytybb gjnjrf: %d\n", val1);
    	return EXIT_FAILURE; 
    }
    
    // Выполнение кода в главном потоке после завершения дочернего
    print_lines();
    
    return 0;
}

