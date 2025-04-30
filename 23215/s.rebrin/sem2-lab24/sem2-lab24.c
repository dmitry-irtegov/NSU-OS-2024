#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>

#define a 10
#define b 10
#define c 10
#define m 10

sem_t A;
sem_t B;
sem_t C;
sem_t module;

void* thread_body_a(void* param) {

    while (1) {
        sleep(1);
        sem_post(&A);
        write(0, "Part A is manufactured\n", 23);
    }
    return NULL;
}
void* thread_body_b(void* param) {

    while (1) {
        sleep(2);
        sem_post(&B);
        write(0, "Part B is manufactured\n", 23);
    }
    return NULL;
}
void* thread_body_c(void* param) {

    while (1) {
        sleep(3);
        sem_post(&C);
        write(0, "Part C is manufactured\n", 23);
    }
    return NULL;
}
void* thread_body_module(void* param) {

    while (1) {
        sleep(1);
        sem_wait(&A);
        sem_wait(&B);
        sem_post(&module);
        write(0, "Part m is manufactured\n", 23);
    }
    return NULL;
}
void* thread_body_widget(void* param) {

    while (1) {
        sleep(2);
        sem_wait(&module);
        sem_wait(&C);
        write(0, "Part w is manufactured\n", 23);
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    pthread_t thread;
    int code;

    sem_init(&A, 0, a);
    sem_init(&B, 0, b);
    sem_init(&C, 0, c);
    sem_init(&module, 0, m);

    code = pthread_create(&thread, NULL, thread_body_a, NULL);
    if (code != 0) {
        char buf[256];
        strerror_r(code, buf, sizeof buf);
        fprintf(stderr, "%s: creating thread: %s\n", argv[0], buf);
        exit(1);
    }
    code = pthread_create(&thread, NULL, thread_body_b, NULL);
    if (code != 0) {
        char buf[256];
        strerror_r(code, buf, sizeof buf);
        fprintf(stderr, "%s: creating thread: %s\n", argv[0], buf);
        exit(1);
    }
    code = pthread_create(&thread, NULL, thread_body_c, NULL);
    if (code != 0) {
        char buf[256];
        strerror_r(code, buf, sizeof buf);
        fprintf(stderr, "%s: creating thread: %s\n", argv[0], buf);
        exit(1);
    }
    code = pthread_create(&thread, NULL, thread_body_module, NULL);
    if (code != 0) {
        char buf[256];
        strerror_r(code, buf, sizeof buf);
        fprintf(stderr, "%s: creating thread: %s\n", argv[0], buf);
        exit(1);
    }
    code = pthread_create(&thread, NULL, thread_body_widget, NULL);
    if (code != 0) {
        char buf[256];
        strerror_r(code, buf, sizeof buf);
        fprintf(stderr, "%s: creating thread: %s\n", argv[0], buf);
        exit(1);
    }

    sem_destroy(&A);
    sem_destroy(&B);
    sem_destroy(&C);
    sem_destroy(&module);


    pthread_exit(NULL);
}

