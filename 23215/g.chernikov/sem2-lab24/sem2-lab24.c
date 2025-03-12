#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

/*
    детали A = 1 sec; B = 2 sec; C = 3 sec; AB = A + B; widget = AB + C;
*/

sem_t sem_a, sem_b, sem_c, sem_ab;

void* create_A() {
    while(1) {
        sleep(1);
        sem_post(&sem_a);
        printf("A done!\n");
    }
}

void* create_B() {
    while(1) {
        sleep(2);
        sem_post(&sem_b);
        printf("B done!\n");
    }
}

void* create_C() {
    while(1) {
        sleep(3);
        sem_post(&sem_c);
        printf("C done!\n");
    }
}

void* create_AB() {
    while(1) {
        sem_wait(&sem_a);
        sem_wait(&sem_b);
        sem_post(&sem_ab);
        printf("AB done!\n");
    }
}

void* create_widget() {
    while(1) {
        sem_wait(&sem_c);
        sem_wait(&sem_ab);
        printf("widget done!\n");
    }
}

int main() {
    pthread_t threadA;
    pthread_t threadB;
    pthread_t threadC;
    pthread_t threadAB;
    pthread_t thread_widget;

    sem_init(&sem_a, 0, 0);
    sem_init(&sem_b, 0, 0);
    sem_init(&sem_c, 0, 0);
    sem_init(&sem_ab, 0, 0);

    pthread_create(&threadA, NULL, create_A, NULL);
    pthread_create(&threadB, NULL, create_B, NULL);
    pthread_create(&threadAB, NULL, create_AB, NULL);
    pthread_create(&thread_widget, NULL, create_widget, NULL);

    pthread_exit(NULL);

}