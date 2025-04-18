#include <stdio.h>
#include <pthread.h>

void* print_messages(void* arg) {
    for (int i = 0; i < 10; i++) {
        printf("Дочерний поток: строка %d\n", i + 1);
    }
    return NULL;
}

int main() {
    pthread_t thread;

    pthread_create(&thread, NULL, print_messages, NULL);

    for (int i = 0; i < 10; i++) {
        printf("Родительский поток: строка %d\n", i + 1);
    }

    return 0;
}
