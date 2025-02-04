#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Функция, выполняемая в новой нити
void *thread_function(void *arg) {
    for (int i = 0; i < 10; i++) {
        printf("[Нить] Сообщение %d\n", i + 1);
    }
    return NULL;
}

int main() {
    pthread_t thread;
    int err;

    // Создание новой нити
    err = pthread_create(&thread, NULL, thread_function, NULL);
    if (err != 0) {
        fprintf(stderr, "Ошибка при создании нити: %s\n", strerror(err));
        return EXIT_FAILURE;
    }

    // Код основной нити
    for (int i = 0; i < 10; i++) {
        printf("[Основная нить] Сообщение %d\n", i + 1);
    }

    // Ожидание завершения созданной нити
    pthread_join(thread, NULL);

    return EXIT_SUCCESS;
}