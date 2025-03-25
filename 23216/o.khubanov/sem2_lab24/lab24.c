#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

// Семафоры-счетчики
sem_t sem_A, sem_B, sem_C, sem_Module;

// Потоки-изготовители
void* make_A() {
    while (1) {
        sleep(1); // Время изготовления A
        sem_post(&sem_A); // Увеличить количество деталей A
        printf("Изготовлена деталь A\n");
    }
}

void* make_B() {
    while (1) {
        sleep(2); // Время изготовления B
        sem_post(&sem_B); // Увеличить количество деталей B
        printf("Изготовлена деталь B\n");
    }
}

void* make_C() {
    while (1) {
        sleep(3); // Время изготовления C
        sem_post(&sem_C); // Увеличить количество деталей C
        printf("Изготовлена деталь C\n");
    }
}

// Сборка модуля из A и B
void* assemble_module() {
    while (1) {
        sem_wait(&sem_A);
        sem_wait(&sem_B);
        printf("Собран модуль из A и B\n");
        sem_post(&sem_Module);
    }
}

// Сборка винтика из модуля и C
void* assemble_widget() {
    int count = 0;
    while (1) {
        sem_wait(&sem_Module);
        sem_wait(&sem_C);
        count++;
        printf("🛠️  Собран винтик #%d\n", count);
    }
}

int main() {
    // Инициализация семафоров (начальное значение 0)
    sem_init(&sem_A, 0, 0);
    sem_init(&sem_B, 0, 0);
    sem_init(&sem_C, 0, 0);
    sem_init(&sem_Module, 0, 0);

    pthread_t a_thread, b_thread, c_thread, module_thread, widget_thread;

    // Запуск потоков
    pthread_create(&a_thread, NULL, make_A, NULL);
    pthread_create(&b_thread, NULL, make_B, NULL);
    pthread_create(&c_thread, NULL, make_C, NULL);
    pthread_create(&module_thread, NULL, assemble_module, NULL);
    pthread_create(&widget_thread, NULL, assemble_widget, NULL);

    // Присоединение (не обязательно, если программа работает бесконечно)
    pthread_join(a_thread, NULL);
    pthread_join(b_thread, NULL);
    pthread_join(c_thread, NULL);
    pthread_join(module_thread, NULL);
    pthread_join(widget_thread, NULL);

    return 0;
}

