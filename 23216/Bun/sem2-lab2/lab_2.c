#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define handle_error(err, msg) do { errno = err; perror(msg); exit(EXIT_FAILURE); } while(0)
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
        handle_error(res, "error creating thread");
    }

    if ((res = pthread_join(thread, NULL)) != 0) {
        handle_error(res, "error thread_join");
    }
    thread_function("from parent");
    return 0;

}
