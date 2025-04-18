#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>


void * thread_body(void * param) {
    for (int i = 0; i < 10; i++) {
        printf("Child\n");
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    pthread_t thread;

    if (pthread_create(&thread, NULL, thread_body, NULL) != 0) {
        perror("error pthread create");
        return 1;
    }
    
    // Ожидаем завершения дочернего потока
    pthread_join(thread, NULL);
    
    for (int i = 0; i < 10; i++) {
        printf("Parent\n");
    }
    
    return 0;
}
