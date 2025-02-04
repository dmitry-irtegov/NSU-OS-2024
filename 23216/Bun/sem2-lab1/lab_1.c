#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
void* thread_function() {
    for (int i = 0; i < 10; i++) {
        printf("Hello World\n");
    }
    pthread_exit(NULL);
    return NULL;
}
int main() {
    pthread_t thread;
    int res;
    if ((res = pthread_create(&thread, NULL, thread_function, NULL)) != 0) {
        char err[256];
        strerror_r(res, err, sizeof(err));
        fprintf(stderr, "Error creating thread: %s\n", err);
        exit(EXIT_FAILURE);
    }
    thread_function();

    pthread_exit(NULL);
}
