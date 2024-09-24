#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

void *thread_function(void *arg) {
    printf("Child\n");
    for (int i = 0; i < 10; i++) {
        printf("text %d\n", i + 1);
    }
    return NULL;
}

int main() {
    pthread_t thread;
 
    if (pthread_create(&thread, NULL, thread_function, NULL) != 0) {
        perror("error pthread create");
        return 1;
    }

    pthread_join(thread, NULL);

    printf("Parent\n");
    for (int i = 0; i < 10; i++) {
        printf("text %d\n", i + 1);
    }

    return 0;
}