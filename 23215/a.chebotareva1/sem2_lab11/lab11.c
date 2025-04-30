#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <stdatomic.h>
#define M 3

// Объявляем "кольцо" из трех мьютексов
pthread_mutex_t mut[M];
atomic_int turn = 0;

void *thread_func(void *arg) {
    // Блокируем начальный мьютекс для дочерней нити
    int child_cur = 2;
    int ret = pthread_mutex_lock(&mut[child_cur]);
    if (ret != 0) {
        fprintf(stderr, "Child thread: mutex lock error: %d\n", ret);
        pthread_exit(NULL);
    }
    atomic_store(&turn, 1);
    for (int i = 1; i <= 10; i++) {
        // Блокируем следующий мьютекс в кольце
        child_cur = (child_cur + 1) % M;
        ret = pthread_mutex_lock(&mut[child_cur]);
        if (ret != 0) {
            fprintf(stderr, "Child thread: mutex lock error: %d\n", ret);
            pthread_exit(NULL);
        }
        // Выводим строку дочерней нити
        printf("Child line num %d\n", i);
        // Разблокируем предыдущий нашему мьютексу (блокировали его в прошлый раз)
        ret = pthread_mutex_unlock(&mut[(child_cur + M - 1) % M]);
        if (ret != 0) {
            fprintf(stderr, "Child thread: mutex unlock error: %d\n", ret);
            pthread_exit(NULL);
        }
    }
    // Разблокируем последний мьютекс
    ret = pthread_mutex_unlock(&mut[child_cur]);
    if (ret != 0) {
        fprintf(stderr, "Child thread: mutex unlock error: %d\n", ret);
        pthread_exit(NULL);
    }
    pthread_exit(NULL);
}

int main() {
    pthread_t thread;
    pthread_mutexattr_t mutex_attr;

    // Инициализируем атрибуты мьютекса с типом PTHREAD_MUTEX_ERRORCHECK
    if (pthread_mutexattr_init(&mutex_attr) != 0) {
        fprintf(stderr, "Mutex attribute init error\n");
        return EXIT_FAILURE;
    }
    if (pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_ERRORCHECK) != 0) {
        fprintf(stderr, "Mutex attribute settype error\n");
        return EXIT_FAILURE;
    }

    // Инициализируем мьютексы
    for (int i = 0; i < M; i++) {
        if (pthread_mutex_init(&mut[i], &mutex_attr) != 0) {
            fprintf(stderr, "Mutex init error\n");
            for (int j = 0; j < i; j++) {
                pthread_mutex_destroy(&mut[j]);
            }
            pthread_mutexattr_destroy(&mutex_attr);
            return EXIT_FAILURE;
        }
    }

    // Блокируем начальный мьютекс для родительской нити
    int parent_cur = 0;
    int ret = pthread_mutex_lock(&mut[parent_cur]);
    if (ret != 0) {
        fprintf(stderr, "Parent thread: mutex lock error: %d\n", ret);
        for (int j = 0; j < M; j++) {
            pthread_mutex_destroy(&mut[j]);
        }
        pthread_mutexattr_destroy(&mutex_attr);
        return EXIT_FAILURE;
    }

    // Создаем дочернюю нить
    if (pthread_create(&thread, NULL, thread_func, NULL) != 0) {
        fprintf(stderr, "Error with thread creating\n");
        for (int j = 0; j < M; j++) {
            pthread_mutex_destroy(&mut[j]);
        }
        pthread_mutexattr_destroy(&mutex_attr);
        return EXIT_FAILURE;
    }

    for (int i = 1; i <= 10; i++) {
        while (!atomic_load(&turn));
        // Блокируем следующий мьютекс в кольце
        parent_cur = (parent_cur + 1) % M;
        ret = pthread_mutex_lock(&mut[parent_cur]);
        if (ret != 0) {
            fprintf(stderr, "Parent thread: mutex lock error: %d\n", ret);
            return EXIT_FAILURE;
        }
        // Выводим строку родительской нити
        printf("Parent line num %d\n", i);
        // Разблокируем предыдущий нашему мьютексу (блокировали его в прошлый раз)
        ret = pthread_mutex_unlock(&mut[(parent_cur + M - 1) % M]);
        if (ret != 0) {
            fprintf(stderr, "Parent thread: mutex unlock error: %d\n", ret);
            return EXIT_FAILURE;
        }
    }
    // Разблокируем последний мьютекс
    ret = pthread_mutex_unlock(&mut[parent_cur]);
    if (ret != 0) {
        fprintf(stderr, "Parent thread: mutex unlock error: %d\n", ret);
        return EXIT_FAILURE;
    }

    // Ожидаем завершения дочерней нити
    if (pthread_join(thread, NULL) != 0) {
        fprintf(stderr, "Thread join error\n");
        return EXIT_FAILURE;
    }
    // Уничтожаем мьютексы и атрибуты
    for (int i = 0; i < M; i++) {
        if (pthread_mutex_destroy(&mut[i]) != 0) {
            fprintf(stderr, "Mutex %d destroy error\n", i);
            return EXIT_FAILURE;
        }
    }
    if (pthread_mutexattr_destroy(&mutex_attr) != 0) {
        fprintf(stderr, "Mutex attribute destroy error\n");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}