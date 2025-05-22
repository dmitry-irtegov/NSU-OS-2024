#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>

sem_t sem_A, sem_B, sem_C, sem_module;
sem_t sem_stop; 

void handle_sigint(int sig) {
    printf("\n[System] Завершение по Ctrl+C...\n");
    sem_post(&sem_stop); 
}

int should_stop() {
    int val;
    sem_getvalue(&sem_stop, &val);
    return val > 0;
}

void *make_A(void *arg) {
    while (!should_stop()) {
        sleep(1);
        sem_post(&sem_A);
        printf("[A] Изготовлена деталь A\n");
    }
    return NULL;
}

void *make_B(void *arg) {
    while (!should_stop()) {
        sleep(2);
        sem_post(&sem_B);
        printf("[B] Изготовлена деталь B\n");
    }
    return NULL;
}

void *make_C(void *arg) {
    while (!should_stop()) {
        sleep(3);
        sem_post(&sem_C);
        printf("[C] Изготовлена деталь C\n");
    }
    return NULL;
}

void *assemble_module(void *arg) {
    while (!should_stop()) {
        sem_wait(&sem_A);
        sem_wait(&sem_B);
        printf("[Module] Собран модуль из A и B\n");
        sem_post(&sem_module);
    }
    return NULL;
}

void *assemble_widget(void *arg) {
    int count = 0;
    while (!should_stop()) {
        sem_wait(&sem_module);
        sem_wait(&sem_C);
        count++;
        printf("[Widget] Винтик #%d собран (Module + C)\n", count);
       
    }
    return NULL;
}

int main() {
    signal(SIGINT, handle_sigint);

    pthread_t threads[5];

    sem_init(&sem_A, 0, 0);
    sem_init(&sem_B, 0, 0);
    sem_init(&sem_C, 0, 0);
    sem_init(&sem_module, 0, 0);
    sem_init(&sem_stop, 0, 0);

    pthread_create(&threads[0], NULL, make_A, NULL);
    pthread_create(&threads[1], NULL, make_B, NULL);
    pthread_create(&threads[2], NULL, make_C, NULL);
    pthread_create(&threads[3], NULL, assemble_module, NULL);
    pthread_create(&threads[4], NULL, assemble_widget, NULL);

    int i = 0;
    for (i; i < 5; i++) {
        pthread_join(threads[i], NULL);
    }

    sem_destroy(&sem_A);
    sem_destroy(&sem_B);
    sem_destroy(&sem_C);
    sem_destroy(&sem_module);
    sem_destroy(&sem_stop);

    printf("[System] Завершено корректно.\n");
    return 0;
}
