#include <stdio.h>     // Для puts
#include <stdlib.h>    // Для NULL
#include <stddef.h>    // Для NULL (дополнительно)
#include <pthread.h>   // Для потоков

void* print_lines() {
    for (int i = 0; i < 10; i++) {
        puts("Текст: строка");
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
    
    // Выполнение кода в главном потоке
    print_lines(NULL);
    
    // Ожидание завершения дочернего потока
    pthread_exit(NULL);
    
    return 0;
}

