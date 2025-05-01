#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#define NUM_ITERATIONS 5

pthread_mutex_t mutex;
int turn = 0; // 0 - родитель, 1 - дочерний поток

void* parent_thread(void* arg) {
    for (int i = 0; i < NUM_ITERATIONS; i++) {

        pthread_mutex_lock(&mutex);

        if (turn == 0) {
            printf("Родительская нить, строка %d\n", i + 1);
            turn = 1;  
        }

        pthread_mutex_unlock(&mutex);

        usleep(100000); 
    }
    return NULL;
}

void* child_thread(void* arg) {
    for (int i = 0; i < NUM_ITERATIONS; i++) {

        pthread_mutex_lock(&mutex);

        if (turn == 1) {
            printf("Дочерняя нить, строка %d\n", i + 1);
            turn = 0;
        }

        pthread_mutex_unlock(&mutex);

        usleep(100000); 
    }
    return NULL;
}

int main() {
    pthread_t parent, child;

    if (pthread_mutex_init(&mutex, NULL) != 0) {
        printf("Не удалось инициализировать мутекс\n");
        return -1;
    }

    if (pthread_create(&parent, NULL, parent_thread, NULL) != 0) {
        printf("Не удалось создать родительский поток\n");
        return -1;
    }

    if (pthread_create(&child, NULL, child_thread, NULL) != 0) {
        printf("Не удалось создать дочерний поток\n");
        return -1;
    }

    pthread_join(parent, NULL);
    pthread_join(child, NULL);

    pthread_mutex_destroy(&mutex);

    return 0;
}
