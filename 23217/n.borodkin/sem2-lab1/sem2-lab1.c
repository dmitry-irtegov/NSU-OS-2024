#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void* thread_function(void* arg) {
    for (int i = 0; i < 10; i++) {
        printf("Дочерний поток: строка %d\n", i + 1);
    }
    return NULL;
}

int main() {
    pthread_t thread;
    
    if (pthread_create(&thread, NULL, thread_function, NULL) != 0) {
        perror("Ошибка при создании потока");
        return EXIT_FAILURE;
    }
    
    if (pthread_join(thread, NULL) != 0) {
        perror("Ошибка при ожидании потока");
        return EXIT_FAILURE;
    }
    
    for (int i = 0; i < 10; i++) {
        printf("Родительский поток: строка %d\n", i + 1);
    }
    
    return EXIT_SUCCESS;
}
