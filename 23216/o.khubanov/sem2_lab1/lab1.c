#include <stdio.h>    
#include <stdlib.h>   
#include <pthread.h>  

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
    pthread_attr_init(&attr);

    // Создание потока с установленными атрибутами
    int val = pthread_create(&thread, &attr, print_lines, NULL);
    if (val != 0) {
        fprintf(stderr, "Ошибка при создании потока: %d\n", val);
        pthread_attr_destroy(&attr); // Освобождаем атрибуты перед выходом
        return EXIT_FAILURE;
    }

    pthread_attr_destroy(&attr);

    // Выполнение кода в главном потоке
    print_lines(NULL);

    // Завершаем главный поток, позволяя дочернему завершиться
    pthread_exit(NULL);
}

