#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

sem_t part_a, part_b, part_c, module;

void *a_producer(void *arg) {
    while (1) {
        sleep(1);
        sem_post(&part_a);
        printf("part A was produced\n");
    }
}
void *b_producer(void *arg) {
    while (1) {
        sleep(2);
        sem_post(&part_b);
        printf("part B was produced\n");
    }
}
void *c_producer(void *arg) {
    while (1) {
        sleep(3);
        sem_post(&part_c);
        printf("part C was produced\n");
    }
}

void *module_producer(void *arg) {
    while (1) {
        sem_wait(&part_a);
        sem_wait(&part_b);
        sem_post(&module);
        printf("module was produced\n");
    }
}

void *widget_producer(void *arg) {
    while (1) {
        sem_wait(&module);
        sem_wait(&part_c);
        printf("widget was produced\n");
    }
}

int main() {
    sem_init(&part_a, 0, 0);
    sem_init(&part_b, 0, 0);
    sem_init(&part_c, 0, 0);
    sem_init(&module, 0, 0);

    pthread_t a_prod, b_prod, c_prod, module_prod;

    pthread_create(&a_prod, NULL, a_producer, NULL);
    pthread_create(&b_prod, NULL, b_producer, NULL);
    pthread_create(&c_prod, NULL, c_producer, NULL);
    pthread_create(&module_prod, NULL, module_producer, NULL);

    widget_producer(NULL);
}