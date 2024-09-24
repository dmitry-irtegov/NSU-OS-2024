#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

sem_t sem_A, sem_B, sem_C, sem_module;

void error_helper(const char *msg)
{
    fprintf(stderr, "ошибка: %s \n", msg);
    exit(EXIT_FAILURE);
}

void *detail_A(void *arg)
{
    while (1)
    {
        sleep(1);
        if (sem_post(&sem_A) == -1) error_helper("sem_post в detail_A");
        printf("[A] изготовлена\n");
    }
    return NULL;
}

void *detail_B(void *arg)
{
    while (1)
    {
        sleep(2);
        if (sem_post(&sem_B) == -1) error_helper("sem_post в detail_B");
        printf("[B] изготовлена\n");
    }
    return NULL;
}

void *detail_C(void *arg)
{
    while (1)
    {
        sleep(3);
        if (sem_post(&sem_C) == -1) error_helper("sem_post в detail_C");
        printf("[C] изготовлена\n");
    }
    return NULL;
}

void* build_module(void* arg) {
    while(1) {
        if (sem_wait(&sem_A) == -1) error_helper("sem_wait в build_module (A)");
        if (sem_wait(&sem_B) == -1) error_helper("sem_wait в build_module (B)");
        
        sleep(1);
        
        if (sem_post(&sem_module) == -1) error_helper("sem_post в build_module");
        printf("[MODULE] A+B собран\n");
    }
    return NULL;
}

void* build_widget(void* arg) {
    int widget_count = 0;
    while(1) {
        if (sem_wait(&sem_module) == -1) error_helper("sem_wait в build_widget (module)");
        if (sem_wait(&sem_C) == -1) error_helper("sem_wait в build_widget (C)");
        
        sleep(1);

        printf("[WIDGET] #%d (Module+C) готов\n", ++widget_count);
    }
    return NULL;
}

int init_semaphores()
{
    if (sem_init(&sem_A, 0, 0) == -1) return -1;
    if (sem_init(&sem_B, 0, 0) == -1) return -1;
    if (sem_init(&sem_C, 0, 0) == -1) return -1;
    if (sem_init(&sem_module, 0, 0) == -1) return -1;
    return 0;
} 

void destroy_semaphores()
{
    sem_destroy(&sem_A);
    sem_destroy(&sem_B);
    sem_destroy(&sem_C);
    sem_destroy(&sem_module);
}

int main()
{
    if (init_semaphores() == -1) error_helper("ошибка инициализации семафоров");

    pthread_t threads[5];
    if (pthread_create(&threads[0], NULL, detail_A, NULL) != 0) error_helper("pthread_create A");
    if (pthread_create(&threads[1], NULL, detail_B, NULL) != 0) error_helper("pthread_create B");
    if (pthread_create(&threads[2], NULL, detail_C, NULL) != 0) error_helper("pthread_create C");
    if (pthread_create(&threads[3], NULL, build_module, NULL) != 0) error_helper("pthread_create module");
    if (pthread_create(&threads[4], NULL, build_widget, NULL) != 0) error_helper("pthread_create widget");

    for (int i = 0; i < 5; i++) {
        if (pthread_join(threads[i], NULL) != 0) error_helper("pthread_join");
    }

    destroy_semaphores();

    return 0;
}
