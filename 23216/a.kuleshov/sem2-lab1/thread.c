#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Функция вывода 10 строк
void ten_str(char* name) {
    for (int i = 0; i < 10; i++) {
        printf("[%s] Сообщение %d\n",name, i + 1);
    }
}

// Функция, выполняемая в новой нити
void *thread_function() {
    ten_str("Нить");
    return NULL;
}

// Функция вывода ошибки
void err_print(int error) {
    fprintf(stderr, "Ошибка: %s\n", strerror(error));
    _exit(EXIT_FAILURE);
}

int main() {
    pthread_t thread;
    int err;

    // Создание новой нити
    err = pthread_create(&thread, NULL, thread_function, NULL);
    if (err != 0) {
        err_print(err);
    }

    // Код основной нити
    ten_str("Основная нить");

    // Ожидание завершения созданной нити
    err = pthread_join(thread, NULL);
    if (err != 0) {
        err_print(err);
    }

    return EXIT_SUCCESS;
}