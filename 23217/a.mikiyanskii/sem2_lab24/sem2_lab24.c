#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

sem_t sem_A, sem_B, sem_C, sem_module;

void* produce_A(void* arg) {
    while (1) {
        sleep(1);  
        sem_post(&sem_A);
        printf("Деталь A готова\n");
    }
    return NULL;
}

void* produce_B(void* arg) {
    while (1) {
        sleep(2); 
        sem_post(&sem_B);
        printf("Деталь B готова\n");
    }
    return NULL;
}

void* produce_C(void* arg) {
    while (1) {
        sleep(3);
        sem_post(&sem_C);
        printf("Деталь C готова\n");
    }
    return NULL;
}

void* assemble_module(void* arg) {
    while (1) {
        sem_wait(&sem_A);
        sem_wait(&sem_B);
        printf("Модуль собран из A и B\n");
        sem_post(&sem_module);
    }
    return NULL;
}

void* assemble_widget(void* arg) {
    while (1) {
        sem_wait(&sem_module);
        sem_wait(&sem_C);
        printf("Винтик (Widget) собран!\n\n");
    }
    return NULL;
}

int main() {
    sem_init(&sem_A, 0, 0);
    sem_init(&sem_B, 0, 0);
    sem_init(&sem_C, 0, 0);
    sem_init(&sem_module, 0, 0);

    pthread_t threads[5];
    pthread_create(&threads[0], NULL, produce_A, NULL);
    pthread_create(&threads[1], NULL, produce_B, NULL);
    pthread_create(&threads[2], NULL, produce_C, NULL);
    pthread_create(&threads[3], NULL, assemble_module, NULL);
    pthread_create(&threads[4], NULL, assemble_widget, NULL);

    for (int i = 0; i < 5; i++) {
        pthread_join(threads[i], NULL);
    }

    sem_destroy(&sem_A);
    sem_destroy(&sem_B);
    sem_destroy(&sem_C);
    sem_destroy(&sem_module);

    return 0;
}
