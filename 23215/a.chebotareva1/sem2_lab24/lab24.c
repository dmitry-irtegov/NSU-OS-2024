#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>

sem_t a, b, c, m;

void* thread_a() {
    while (1) {
        sleep(1);
        sem_post(&a);
        printf("Part A was created\n");
    }
    return NULL;
}
void* thread_b() {
    while (1) {
        sleep(2);
        sem_post(&b);
        printf("Part B was created\n");
    }
    return NULL;
}
void* thread_c() {
    while (1) {
        sleep(3);
        sem_post(&c);
        printf("Part C was created\n");
    }
    return NULL;
}
void* thread_module() {
    while (1) {
        sem_wait(&a);
        sem_wait(&b);
        sem_post(&m);
        printf("Module was created\n");
    }
    return NULL;
}
void* create_widget() {
    while (1) {
        sem_wait(&m);
        sem_wait(&c);
        printf("Widget was created\n");
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    pthread_t threads[5];
    sem_init(&a, 0, 0);
    sem_init(&b, 0, 0);
    sem_init(&c, 0, 0);
    sem_init(&m, 0, 0);

    if (pthread_create(&threads[0], NULL, thread_a, NULL) != 0) {
        fprintf(stderr, "Failed creating part a thread");
        return EXIT_FAILURE;
    }
    if (pthread_create(&threads[1], NULL, thread_b, NULL) != 0) {
        fprintf(stderr, "Failed creating part b thread");
        return EXIT_FAILURE;
    }
    if (pthread_create(&threads[2], NULL, thread_c, NULL) != 0) {
        fprintf(stderr, "Failed creating part c thread");
        return EXIT_FAILURE;
    }
    if (pthread_create(&threads[3], NULL, thread_module, NULL) != 0) {
        fprintf(stderr, "Failed creating module thread");
        return EXIT_FAILURE;
    }
    create_widget();
    return EXIT_SUCCESS;
}