#include <stdio.h>
#include <semaphore.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

sem_t sem_a, sem_b, sem_c, sem_module;

void* produce_a(void* param) {
    while(1) {
        sleep(1);
        printf("Произведена деталь A\n");
        sem_post(&sem_a);
    }
}

void* produce_b(void* param) {
    while(1) {
        sleep(2);
        printf("Произведена деталь B\n");
        sem_post(&sem_b);
    }
}

void* produce_c(void* param) {
    while(1) {
        sleep(3);
        printf("Произведена деталь C\n");
        sem_post(&sem_c);
    }
}

void* produce_module(void* param) {
    while(1) {
        sem_wait(&sem_a);
        sem_wait(&sem_b);
        printf("Произведен модуль\n");
        sem_post(&sem_module);
    }
}

void* produce_widget(void* param) {
    while(1) {
        sem_wait(&sem_c);
        sem_wait(&sem_module);
        printf("Произведен винтик\n");
    }
}

int main() {
    sem_init(&sem_a, 0, 0);
    sem_init(&sem_b, 0, 0);
    sem_init(&sem_c, 0, 0);
    sem_init(&sem_module, 0, 0);

    pthread_t thread_a, thread_b, thread_c, thread_module, thread_widget;

    pthread_create(&thread_a, NULL, produce_a, NULL);
    pthread_create(&thread_b, NULL, produce_b, NULL);
    pthread_create(&thread_c, NULL, produce_c, NULL);
    pthread_create(&thread_module, NULL, produce_module, NULL);
    pthread_create(&thread_widget, NULL, produce_widget, NULL);

    pthread_exit(NULL);

    return 0;
}
