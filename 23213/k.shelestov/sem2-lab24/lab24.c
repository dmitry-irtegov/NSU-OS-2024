#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <signal.h>

sem_t sem_A, sem_B, sem_C, sem_Module;
pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;
volatile int count_A, count_B, count_C, count_Module;

void *signal_handler_thread(void *arg) {
    sigset_t set;
    size_t len;
    int sig;

    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    
    while(1) {
        sigwait(&set, &sig);
        pthread_mutex_lock(&counter_mutex);
        len = snprintf(NULL, 0, 
                      "Детали A: %d, B: %d, C: %d, Модули: %d\n", 
                      count_A, count_B, count_C, count_Module); 
        char buffer[len+1];  
        snprintf(buffer, len+1, 
            "Детали A: %d, B: %d, C: %d, Модули: %d\n", 
            count_A, count_B, count_C, count_Module); 
        write(STDOUT_FILENO, buffer, len);
        pthread_mutex_unlock(&counter_mutex);
        _exit(EXIT_FAILURE);
    }
}

void *produce_A(void *arg) {
    char msg[64] = "Деталь A сделана\n";
    while (1) {
        sleep(1);
        sem_post(&sem_A);
        pthread_mutex_lock(&counter_mutex);
        count_A++;
        pthread_mutex_unlock(&counter_mutex);
        write(STDOUT_FILENO, msg, 64);
    }
}

void *produce_B(void *arg) {
    const char msg[64] = "Деталь B сделана\n";
    while (1) {
        sleep(2); 
        sem_post(&sem_B);
        pthread_mutex_lock(&counter_mutex);
        count_B++;
        pthread_mutex_unlock(&counter_mutex);
        write(STDOUT_FILENO, msg, 64);
    }
}

void *produce_C(void *arg) {
    char msg[64] = "Деталь C сделана\n";
    while (1) {
        sleep(3);  
        sem_post(&sem_C);
        pthread_mutex_lock(&counter_mutex);
        count_C++;
        pthread_mutex_unlock(&counter_mutex);
        write(STDOUT_FILENO, msg, 64);
    }
}

void *assemble_module(void *arg) {
    const char msg[64] = "Собран модуль AB\n";
    while (1) {
        sem_wait(&sem_A);
        sem_wait(&sem_B);
        pthread_mutex_lock(&counter_mutex);
        count_A--;
        count_B--;
        count_Module++;
        pthread_mutex_unlock(&counter_mutex);
        sem_post(&sem_Module);
        write(STDOUT_FILENO, msg, 64);
    }
}

void *assemble_widget() {
    char msg[64] = "Собран винтик\n";
    while (1) {
        sem_wait(&sem_Module);
        sem_wait(&sem_C);
        pthread_mutex_lock(&counter_mutex);
        count_Module--;
        count_C--;
        pthread_mutex_unlock(&counter_mutex);
        write(STDOUT_FILENO, msg, 64);
    }
}

int main() {
    pthread_t thread_A, thread_B, thread_C, thread_Module, thread_Signal;

    sigset_t set;

    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    pthread_sigmask(SIG_BLOCK, &set, NULL);

    pthread_create(&thread_Signal, NULL, signal_handler_thread, NULL);

    sem_init(&sem_A, 0, 0);
    sem_init(&sem_B, 0, 0);
    sem_init(&sem_C, 0, 0);
    sem_init(&sem_Module, 0, 0);

    pthread_create(&thread_A, NULL, produce_A, NULL);
    pthread_create(&thread_B, NULL, produce_B, NULL);
    pthread_create(&thread_C, NULL, produce_C, NULL);
    pthread_create(&thread_Module, NULL, assemble_module, NULL);
    assemble_widget();

    return 0; 
}
