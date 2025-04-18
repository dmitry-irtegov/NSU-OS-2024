#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

sem_t sem_A, sem_B, sem_C, sem_module;

void* makeDetailA(void* arg) {
    sleep(1);
    printf("Detail A created\n");
    sem_post(&sem_A);  
    return NULL;
}

void* makeDetailB(void* arg) {
    sleep(2);
    printf("Detail B created\n");
    sem_post(&sem_B);  
    return NULL;
}

void* makeDetailC(void* arg) {
    sleep(3);
    printf("Detail C created\n");
    sem_post(&sem_C);  
    return NULL;
}

void* module(void* arg) {
    sem_wait(&sem_A);  
    sem_wait(&sem_B); 
    printf("Module created\n");
    sem_post(&sem_module); 
    return NULL;
}

void* makeWidget(void* arg) {
    sem_wait(&sem_module);  
    sem_wait(&sem_C); 
    printf("Widget created\n");
    return NULL;
}

int main() {
    pthread_t tA, tB, tC, tModule, tWidget;

    sem_init(&sem_A, 0, 0);
    sem_init(&sem_B, 0, 0); 
    sem_init(&sem_C, 0, 0);
    sem_init(&sem_module, 0, 0);

    pthread_create(&tA, NULL, makeDetailA, NULL);
    pthread_create(&tB, NULL, makeDetailB, NULL);
    pthread_create(&tC, NULL, makeDetailC, NULL);
    pthread_create(&tModule, NULL, module, NULL);
    pthread_create(&tWidget, NULL, makeWidget, NULL);

    pthread_join(tA, NULL);
    pthread_join(tB, NULL);
    pthread_join(tC, NULL);
    pthread_join(tModule, NULL);
    pthread_join(tWidget, NULL);

    sem_destroy(&sem_A);
    sem_destroy(&sem_B);
    sem_destroy(&sem_C);
    sem_destroy(&sem_module);

    return 0;
}
