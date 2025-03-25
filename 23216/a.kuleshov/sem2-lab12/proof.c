#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

pthread_mutex_t mutex;
pthread_cond_t cond;
int turn = 0; // 0 - основная нить, 1 - дочерняя

// Функция вывода 10 строк
void ten_str(const char* name, int thread_id) {
    for (int i = 0; i < 10; i++) {
        pthread_mutex_lock(&mutex);
        while (turn != thread_id) {
            pthread_cond_wait(&cond, &mutex);
        }
        printf("[%s] Сообщение %d\n", name, i + 1);
        turn = 1 - thread_id;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }
}

// Функция, выполняемая в новой нити
void *thread_function() {
    ten_str("Нить", 1);
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
    pthread_cond_init(&cond, NULL);

    // Создание новой нити
    err = pthread_create(&thread, NULL, thread_function, NULL);
    if (err != 0) {
        err_print(err);
    }

    // Код основной нити
    ten_str("Основная нить", 0);

    // Ожидание завершения созданной нити
    err = pthread_join(thread, NULL);
    if (err != 0) {
        err_print(err);
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
    pthread_mutexattr_destroy(&attr);

    return EXIT_SUCCESS;
}
