#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// Общее количество итераций в ряде Лейбница
#define NUM_STEPS 100000000

// Структура для передачи границ диапазона итераций в поток
typedef struct {
    long start; // начало диапазона
    long end;   // конец диапазона
} ThreadRange;

// Функция потока: вычисляет частичную сумму членов ряда Лейбница
void *compute_pi_partial(void *arg) {
    ThreadRange *range = (ThreadRange *)arg;

    // Выделяем память под частичную сумму, чтобы вернуть её через pthread_exit
    double *partial_sum = malloc(sizeof(double));
    if (!partial_sum) {
        perror("malloc failed");
        pthread_exit(NULL);
    }

    *partial_sum = 0.0;

    // Считаем сумму членов ряда на отрезке [start, end)
    for (long i = range->start; i < range->end; ++i) {
        double term = (i % 2 == 0 ? 1.0 : -1.0) / (2.0 * i + 1.0);
        *partial_sum += term;
    }

    // Освобождаем память под границы диапазона
    free(range);

    // Возвращаем результат как указатель
    pthread_exit((void *)partial_sum);
}

int main(int argc, char *argv[]) {
    // Проверка аргументов командной строки
    if (argc != 2) {
        fprintf(stderr, "Usage: %s num_threads\n", argv[0]);
        return EXIT_FAILURE;
    }

    int num_threads = atoi(argv[1]);
    if (num_threads <= 0) {
        fprintf(stderr, "Number of threads must be positive.\n");
        return EXIT_FAILURE;
    }

    // Выделяем массив для идентификаторов потоков
    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
    if (!threads) {
        perror("malloc failed");
        return EXIT_FAILURE;
    }

    // Размер блока итераций для каждого потока
    long chunk_size = NUM_STEPS / num_threads;
    double pi = 0.0;

    // Запускаем потоки
    for (int i = 0; i < num_threads; ++i) {
        // Выделяем и заполняем структуру с границами диапазона
        ThreadRange *range = malloc(sizeof(ThreadRange));
        if (!range) {
            perror("malloc failed");
            return EXIT_FAILURE;
        }

        range->start = i * chunk_size;
        range->end = (i == num_threads - 1) ? NUM_STEPS : (i + 1) * chunk_size;

        // Создаём поток
        if (pthread_create(&threads[i], NULL, compute_pi_partial, range) != 0) {
            perror("pthread_create failed");
            return EXIT_FAILURE;
        }
    }

    // Собираем результаты от всех потоков
    for (int i = 0; i < num_threads; ++i) {
        void *ret_val;
        if (pthread_join(threads[i], &ret_val) != 0) {
            perror("pthread_join failed");
            return EXIT_FAILURE;
        }

        // Прибавляем частичную сумму к общей
        double *partial = (double *)ret_val;
        pi += *partial;
        free(partial); // Освобождаем память
    }

    // Умножаем на 4 по формуле ряда Лейбница
    pi *= 4.0;

    // Печатаем приближённое значение Пи
    printf("Approximated pi = %.15f\n", pi);

    // Освобождаем память под массив потоков
    free(threads);
    return EXIT_SUCCESS;
}

