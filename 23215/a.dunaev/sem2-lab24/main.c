#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

sem_t sem_A, sem_B, sem_module, sem_C;

int widgets_to_produce;

void* produce_A(void* arg) {
    int count = *(int*)arg;
    for (int i = 0; i < count; i++) {
        sleep(1);
        printf("Деталь A готова\n");
        sem_post(&sem_A);
    }
    return NULL;
}

void* produce_B(void* arg) {
    int count = *(int*)arg;
    for (int i = 0; i < count; i++) {
        sleep(2);
        printf("Деталь B готова\n");
        sem_post(&sem_B);
    }
    return NULL;
}

void* produce_module(void* arg) {
    int count = *(int*)arg;
    for (int i = 0; i < count; i++) {
        sem_wait(&sem_A);
        sem_wait(&sem_B);
        printf("Модуль собран из A и B\n");
        sem_post(&sem_module);
    }
    return NULL;
}

void* produce_C(void* arg) {
    int count = *(int*)arg;
    for (int i = 0; i < count; i++) {
        sleep(3);
        printf("Деталь C готова\n");
        sem_post(&sem_C);
    }
    return NULL;
}

void* produce_widget(void* arg) {
    int count = *(int*)arg;
    for (int i = 0; i < count; i++) {
        sem_wait(&sem_module);
        sem_wait(&sem_C);
        printf("Винтик (widget) собран из модуля и детали C\n");
    }
    return NULL;
}

int main() {
    printf("Введите количество винтиков для изготовления: ");
    scanf("%d", &widgets_to_produce);

    pthread_t threadA, threadB, threadModule, threadC, threadWidget;

    sem_init(&sem_A, 0, 0);
    sem_init(&sem_B, 0, 0);
    sem_init(&sem_module, 0, 0);
    sem_init(&sem_C, 0, 0);

    pthread_create(&threadA, NULL, produce_A, &widgets_to_produce);
    pthread_create(&threadB, NULL, produce_B, &widgets_to_produce);
    pthread_create(&threadModule, NULL, produce_module, &widgets_to_produce);
    pthread_create(&threadC, NULL, produce_C, &widgets_to_produce);
    pthread_create(&threadWidget, NULL, produce_widget, &widgets_to_produce);

    pthread_join(threadA, NULL);
    pthread_join(threadB, NULL);
    pthread_join(threadModule, NULL);
    pthread_join(threadC, NULL);
    pthread_join(threadWidget, NULL);

    sem_destroy(&sem_A);
    sem_destroy(&sem_B);
    sem_destroy(&sem_module);
    sem_destroy(&sem_C);

    printf("Производство завершено\n");

    return 0;
}
