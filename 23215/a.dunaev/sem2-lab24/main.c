#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

sem_t sem_A, sem_B, sem_module, sem_C;
volatile int count_A = 0, count_B = 0, count_C = 0, count_module = 0, count_widget = 0;
volatile int running = 1;

void signal_handler(int signum) {
    running = 0;
}

void* produce_A(void* arg) {
    while(running) {
        sleep(1);
        sem_post(&sem_A);
        count_A++;
        printf("Деталь A готова\n");
    }
    printf("Производство детали A остановлено\n");
    return NULL;
}

void* produce_B(void* arg) {
    while(running) {
        sleep(2);
        sem_post(&sem_B);
        count_B++;
        printf("Деталь B готова\n");
    }
    printf("Производство детали B остановлено\n");
    return NULL;
}

void* produce_module(void* arg) {
    while(running) {
        sem_wait(&sem_A);
        sem_wait(&sem_B);
        sem_post(&sem_module);
        count_module++;
        count_A--;
        count_B--;
        printf("Модуль собран из A и B\n");
    }
    printf("Производство модулей остановлено\n");
    return NULL;
}

void* produce_C(void* arg) {
    while(running) {
        sleep(3);
        sem_post(&sem_C);
        count_C++;
        printf("Деталь C готова\n");
    }
    printf("Производство детали С остановлено\n");
    return NULL;
}

void* produce_widget(void* arg) {
    while(running) {
        sem_wait(&sem_module);
        sem_wait(&sem_C);
        count_widget++;
        count_C--;
        count_module--;
        printf("Винтик (widget) собран из модуля и детали C\n");
    }
    printf("Производство (widget) остановлено\n");
    return NULL;
}

int main() {
    signal(SIGINT, signal_handler);

    pthread_t threadA, threadB, threadModule, threadC, threadWidget;

    sem_init(&sem_A, 0, 0);
    sem_init(&sem_B, 0, 0);
    sem_init(&sem_module, 0, 0);
    sem_init(&sem_C, 0, 0);

    pthread_create(&threadA, NULL, produce_A, NULL);
    pthread_create(&threadB, NULL, produce_B, NULL);
    pthread_create(&threadModule, NULL, produce_module, NULL);
    pthread_create(&threadC, NULL, produce_C, NULL);
    pthread_create(&threadWidget, NULL, produce_widget, NULL);

    pthread_join(threadA, NULL);
    pthread_join(threadB, NULL);
    pthread_join(threadModule, NULL);
    pthread_join(threadC, NULL);
    pthread_join(threadWidget, NULL);

    sem_destroy(&sem_A);
    sem_destroy(&sem_B);
    sem_destroy(&sem_module);
    sem_destroy(&sem_C);

    printf("Всего произведено:\n");
    printf("Деталей A: %d\n", count_A);
    printf("Деталей B: %d\n", count_B);
    printf("Деталей C: %d\n", count_C);
    printf("Модулей: %d\n", count_module);
    printf("Винтиков (widgets): %d\n", count_widget);

    return 0;
}
