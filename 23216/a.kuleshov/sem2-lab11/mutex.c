#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

pthread_mutex_t mutex;
bool turn = true; // true - основная нить false - дочерняя

// Функция вывода 10 строк
void ten_str(const char* name, bool thread_id) {
    int cnt = 0;
    while (cnt < 10) {
        if (thread_id != turn) {
            continue;
        }
        pthread_mutex_lock(&mutex);
        printf("[%s] Сообщение %d\n", name, cnt + 1);
        turn = !turn;
        cnt++;
        pthread_mutex_unlock(&mutex);
    }
}

// Функция, выполняемая в новой нити
void *thread_function() {
    ten_str("Нить", false);
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

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    pthread_mutex_init(&mutex, &attr);

    // Создание новой нити
    err = pthread_create(&thread, NULL, thread_function, NULL);
    if (err != 0) {
        err_print(err);
    }

    // Код основной нити
    ten_str("Основная нить", true);

    // Ожидание завершения созданной нити
    err = pthread_join(thread, NULL);
    if (err != 0) {
        err_print(err);
    }

    pthread_mutex_destroy(&mutex);
    pthread_mutexattr_destroy(&attr);

    return EXIT_SUCCESS;
}
