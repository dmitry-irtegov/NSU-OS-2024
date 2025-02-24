#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>

sem_t semA, semB, semC, semAB;

void* createA(void* argv) {
    while(1) {
        sleep(1);
        sem_post(&semA);    
        printf("new A\n");
    }
}

void* createB(void* argv) {
    while(1) {
        sleep(2);
        sem_post(&semB);    
        printf("new B\n");
    }
}

void* createC(void* argv) {
    while(1) {
        sleep(3);
        sem_post(&semC);    
        printf("new C\n");
    }
}

void* createAB(void* argv) {
    while(1) {
        sem_wait(&semA);
        sem_wait(&semB);
        sem_post(&semAB);
        printf("new AB\n");
    }
}

void* createWidget() {
    while(1) {
        sem_wait(&semAB);
        sem_wait(&semC);
        printf("=== new BOLT ===\n");
    }
}

int main() {
    pthread_t threadA;    
    pthread_t threadB;
    pthread_t threadB2;
    pthread_t threadC;
    pthread_t threadC2;
    pthread_t threadC3;
    pthread_t threadAB;
    
    sem_init(&semA, 0, 0);
    sem_init(&semB, 0, 0);
    sem_init(&semC, 0, 0);
    sem_init(&semAB, 0, 0);

    pthread_create(&threadA, NULL, createA, NULL);
    pthread_create(&threadB, NULL, createB, NULL);
    pthread_create(&threadB2, NULL, createB, NULL);
    pthread_create(&threadB2, NULL, createB, NULL);
    pthread_create(&threadC, NULL, createC, NULL);
    pthread_create(&threadC2, NULL, createC, NULL);
    pthread_create(&threadC3, NULL, createC, NULL);
    pthread_create(&threadAB, NULL, createAB, NULL);
    
    createWidget();    
}
