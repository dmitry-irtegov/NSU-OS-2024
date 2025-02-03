#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void* thread_function(void* arg) {
    for (int i = 0; i < 10; i++) {
        printf("Hello World %s \n", (char*)arg);
    }
    return NULL;
}
int main() {
    pthread_t thread;
    if (pthread_create(&thread, NULL, thread_function, "from child") != 0) {
        perror("error creating thread");
        exit(EXIT_FAILURE);
    }

    if (pthread_join(thread, NULL) != 0) {
        perror("error pthread_join");
        exit(EXIT_FAILURE);
    }
    thread_function("from parent");
    return 0;

}