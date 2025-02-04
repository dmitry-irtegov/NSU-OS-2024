#include <stdio.h>     // Для puts
#include <stdlib.h>    // Для NULL
#include <stddef.h>    // Для NULL (дополнительно)
#include <pthread.h>   // Для потоков

void* print_lines() {
    for (int i = 0; i < 10; i++) {
        printf("%d\n",i);
    }
    return NULL;
}

int main() {
    pthread_t thread;
    
    // Создание нового потока
    val = pthread_create(&thread, NULL, print_lines, NULL);
    if (ret != 0) {
        fprintf(stderr, "Ошибка при создании потока: %d\n", val);
        return EXIT_FAILURE;
    }
    
    // Выполнение кода в главном потоке
    print_lines();
    
    // Ожидание завершения дочернего потока
    pthread_exit(NULL);
    
    return 0;
}

