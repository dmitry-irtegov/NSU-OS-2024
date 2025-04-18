#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>

#define handle_error(en, msg) { errno = en; perror(msg); exit(EXIT_FAILURE); }

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex3 = PTHREAD_MUTEX_INITIALIZER;

int thread_ready = 0;

void print_line(const char* label, int i) {
    if (i < 10) {
        printf("%s: number %d\n", label, i);
    }
}

void* print_lines(void* arg) {
    thread_ready = 1;
    
    pthread_mutex_lock(&mutex3);
    int i = 0;
    while (i < 10) {
        pthread_mutex_lock(&mutex2);
        print_line("Thread", i++);
        pthread_mutex_unlock(&mutex3);
        pthread_mutex_lock(&mutex1);
        print_line("Thread", i++);
        pthread_mutex_unlock(&mutex2);
        pthread_mutex_lock(&mutex3);
        print_line("Thread", i++);
        pthread_mutex_unlock(&mutex1);
    }
    return NULL;
}

int main() {
    int i = 0;
    int res;
    pthread_t thread;
    pthread_mutex_lock(&mutex2);

    res = pthread_create(&thread, NULL, print_lines, NULL);
    if (res != 0) {
        handle_error(res, "pthread_create fail");
    }

    while (!thread_ready) {}

    while (i < 10) {
        pthread_mutex_lock(&mutex1);
        print_line("Parent thread", i++);
        pthread_mutex_unlock(&mutex2);
        pthread_mutex_lock(&mutex3);
        print_line("Parent thread", i++);
        pthread_mutex_unlock(&mutex1);
        pthread_mutex_lock(&mutex2);
        print_line("Parent thread", i++);
        pthread_mutex_unlock(&mutex3);
    }

    pthread_join(thread, NULL);
    pthread_mutex_destroy(&mutex1);
    pthread_mutex_destroy(&mutex2);
    pthread_mutex_destroy(&mutex3);
    pthread_exit(NULL);
}

