#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <semaphore.h>

#define handle_error(en, msg) { errno = en; perror(msg); exit(EXIT_FAILURE); }

sem_t sem1;
sem_t sem2;

void print_line(const char* label, int i) {
    printf("%s: number %d\n", label, i);
}

void* print_lines(void* arg) {
    for (int i = 0; i < 10; i++) {
        sem_wait(&sem2); 
        print_line("Thread", i);
        sem_post(&sem1);  
    }
    return NULL;
}

int main() {
    int res;
    pthread_t thread;

    sem_init(&sem1, 0, 0);  
    sem_init(&sem2, 0, 1); 

    res = pthread_create(&thread, NULL, print_lines, NULL);
    if (res != 0) {
        handle_error(res, "pthread_create fail");
    }

    for (int i = 0; i < 10; i++) {
        sem_wait(&sem1);  
        print_line("Parent thread", i);
        sem_post(&sem2);  
    }

    pthread_join(thread, NULL);
    sem_destroy(&sem1);
    sem_destroy(&sem2);
    pthread_exit(NULL);
}

