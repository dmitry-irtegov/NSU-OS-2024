
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <stdint.h>

#ifndef BLOCK                 /* сколько членов ряда считать между проверками stop */
#   define BLOCK 1000000UL
#endif

/* --- глобальный флаг, который безопасно менять из обработчика SIGINT --- */
static volatile sig_atomic_t stop = 0;

/* --- обработчик SIGINT (Ctrl-C) --- */
static void on_sigint(int sig)
{
    (void)sig;        /* подавляем warning о неиспользуемом параметре */
    stop = 1;         /* только присваивание — async-signal-safe */
}

/* ---------------- структура с параметрами потока ---------------- */
struct thread_arg {
    uint32_t id;         /* номер потока       */
    uint32_t nthreads;   /* всего потоков      */
};

/* ---------------- функция, выполняемая каждым рабочим потоком --- */
static void *worker(void *arg_)
{
    const struct thread_arg *arg = arg_;

    /* частичная сумма потока — выделяем в куче, чтобы вернуть через pthread_exit */
    double *psum = malloc(sizeof *psum);
    if (!psum)
        pthread_exit(NULL);

    *psum = 0.0;

    uint64_t i    = arg->id;         /* первый член ряда для этого потока */
    uint32_t step = arg->nthreads;   /* шаг по индексам */

    while (!stop) {
        for (uint64_t k = 0; k < BLOCK; ++k, i += step) {
            /* ряд Лейбница: π = 4 · Σ (-1)^i / (2i+1)  */
            double term = (i & 1 ? -4.0 : 4.0) / (double)(2 * i + 1);
            *psum += term;
        }
        /* после блока проверяем флаг stop */
    }

    pthread_exit(psum);   /* вернём адрес частичной суммы */
}
/* ---------------------------------------------------------------- */

int main(int argc, char **argv)
{
    /* ---- разбор аргумента командной строки ---- */
    if (argc != 2) {
        fprintf(stderr, "usage: %s <threads>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *end;
    long nthreads = strtol(argv[1], &end, 10);
    if (*end || nthreads < 1) {
        fprintf(stderr, "invalid thread count: %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    /* ---- устанавливаем обработчик SIGINT ---- */
    struct sigaction sa = { .sa_handler = on_sigint };
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    /* ---- создаём массивы дескрипторов потоков и их аргументов ---- */
    pthread_t        *tid  = malloc(sizeof *tid  * nthreads);
    struct thread_arg *arg = malloc(sizeof *arg * nthreads);
    if (!tid || !arg) {
        perror("malloc");
        return EXIT_FAILURE;
    }

    /* ---- стартуем рабочие потоки ---- */
    for (uint32_t i = 0; i < (uint32_t)nthreads; ++i) {
        arg[i] = (struct thread_arg){ .id = i, .nthreads = (uint32_t)nthreads };
        if (pthread_create(&tid[i], NULL, worker, &arg[i])) {
            perror("pthread_create");
            return EXIT_FAILURE;
        }
    }

    /* ---- ждём завершения потоков после Ctrl-C ---- */
    double pi = 0.0;
    for (uint32_t i = 0; i < (uint32_t)nthreads; ++i) {
        void *ret;
        pthread_join(tid[i], &ret);
        if (ret) {
            pi += *(double *)ret;
            free(ret);
        }
    }

    printf("\nπ ≈ %.15f\n", pi);

    free(tid);
    free(arg);
    return EXIT_SUCCESS;
}

