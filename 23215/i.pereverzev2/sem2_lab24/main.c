#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <unistd.h>
#include <pthread.h>

sem_t sem_a;
sem_t sem_b;
sem_t sem_c;
sem_t sem_module;

typedef struct pars_s {
    sem_t* sem;
    int time;
    char type;
} pars;

void* construct_abc(void* params)
{
    pars* paramr = (pars*)params;
    while(1) {
        sleep(paramr->time);
        sem_post(paramr->sem);
        printf("%c was constructed\n", paramr->type);
    }
}

void* construct_module(void* prms) 
{
    while (1)
    {
        sem_wait(&sem_a);
        sem_wait(&sem_b);
        sem_post(&sem_module); 
        printf("module constructed\n");
    }
}

void construct_widget()
{
    while (1)
    {
        sem_wait(&sem_module);
        sem_wait(&sem_c);
        printf("widget constructed\n");
    }
}

int main() {
    sem_init(&sem_a, 0, 0);
    sem_init(&sem_b, 0, 0);
    sem_init(&sem_c, 0, 0);
    sem_init(&sem_module, 0, 0);

    pars pars_a = {&sem_a, 1, 'A'};
    pars pars_b = {&sem_b, 2, 'B'};
    pars pars_c = {&sem_c, 3, 'C'};

    pthread_t thrd_a;
    pthread_t thrd_b;
    pthread_t thrd_c;
    pthread_t thrd_mdl;
    pthread_create(&thrd_a, NULL, construct_abc, &pars_a);
    pthread_create(&thrd_b, NULL, construct_abc, &pars_b);
    pthread_create(&thrd_c, NULL, construct_abc, &pars_c);
    pthread_create(&thrd_mdl, NULL, construct_module, NULL);
    construct_widget();
    return 0;
}
