#include <stdio.h>
#include <pthread.h>

void* thread_function(void* arg) {
    for (int i = 1; i <= 10; i++) {
        printf("thread: %d\n", i);
    }
    return NULL;
}

int main() {
    pthread_t thread_id;

    if (pthread_create(&thread_id, NULL, thread_function, NULL) != 0) {
        perror("thread creation error");
        return 1;
    }

    if (pthread_join(thread_id, NULL) != 0) {
        perror("thread completion waiting error");
        return 1;
    }

    for (int i = 1; i <= 10; i++) {
        printf("parent: %d\n", i);
    }

    return 0;
}

