#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
void* thread_function(void* arg) {
    for (int i = 0; i < 10; i++) {
        printf("Hello World %s \n", (char*)arg);
    }
    return NULL;
}
int main() {
    pthread_t thread;
    int res;
    if ((res = pthread_create(&thread, NULL, thread_function, "from child")) != 0) {
        char err[256];
        strerror_r(res, err, sizeof(err));
        fprintf(stderr, "Error creating thread: %s\n", err);
        exit(EXIT_FAILURE);
    }

    if ((res = pthread_join(thread, NULL)) != 0) {
        char err[256];
        strerror_r(res, err, sizeof(err));
        fprintf(stderr, "Error  pthread_join: %s\n", err);
        exit(EXIT_FAILURE);
    }
    thread_function("from parent");
    return 0;

}
