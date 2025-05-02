#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

sem_t sem_a, sem_b, sem_c, sem_module;

void* produce_a(void* arg) {
    while (1) {
        sleep(1);
        printf("Part A has been produced\n");
        sem_post(&sem_a); 
    }
    return NULL;
}

void* produce_b(void* arg) {
    while (1) {
        sleep(2);
        printf("Part B has been produced\n");
        sem_post(&sem_b);
    }
    return NULL;
}

void* produce_c(void* arg) {
    while (1) {
        sleep(3);
        printf("Part C has been produced\n");
        sem_post(&sem_c);
    }
    return NULL;
}

void* moduling(void* arg) {
    while (1) {
        sem_wait(&sem_a);
        sem_wait(&sem_b);
        printf("The module is assembled from A and B\n");
        sem_post(&sem_module);
    }
    return NULL;
}

void* build_widget(void* arg) {
    while (1) {
        sem_wait(&sem_module);
        sem_wait(&sem_c);
        printf("The widget is assembled\n\n");
    }
    return NULL;
}

int main() {
    pthread_t thread_a, thread_b, thread_c, thread_module, thread_widget;

    sem_init(&sem_a, 0, 0);
    sem_init(&sem_b, 0, 0);
    sem_init(&sem_c, 0, 0);
    sem_init(&sem_module, 0, 0);

    pthread_create(&thread_a, NULL, produce_a, NULL);
    pthread_create(&thread_b, NULL, produce_b, NULL);
    pthread_create(&thread_c, NULL, produce_c, NULL);
    pthread_create(&thread_module, NULL, moduling, NULL);
    pthread_create(&thread_widget, NULL, build_widget, NULL);

    pthread_join(thread_a, NULL);
    pthread_join(thread_b, NULL);
    pthread_join(thread_c, NULL);
    pthread_join(thread_module, NULL);
    pthread_join(thread_widget, NULL);

    sem_destroy(&sem_a);
    sem_destroy(&sem_b);
    sem_destroy(&sem_c);
    sem_destroy(&sem_module);

    return 0;
}
