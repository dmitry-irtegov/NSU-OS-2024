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
    if (pthread_create(&thread, NULL, print_lines, NULL) != 0) {
        perror("Ошибка при создании потока");
        return EXIT_FAILURE;
    }
    
    // Ожидание завершения дочернего потока
    if (pthread_join(thread, NULL) != 0){
    	perror("Ошибкак при присоединениt потокая");
    	return EXIT_FAILURE; 
    }
    
    // Выполнение кода в главном потоке после завершения дочернего
    print_lines();
    
    return 0;
}

