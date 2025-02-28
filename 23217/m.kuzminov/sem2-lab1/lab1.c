#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

void* thread_func(void* param) {
    for(int i = 0; i < 10; i++) {
        printf("%d:new thread\n", i);
    }
}

int main() {
    pthread_t thread;

    int code = pthread_create(&thread, NULL, thread_func, NULL);
    if (code != 0) {
        perror("creating thread");
        exit(1);
    }

    for(int i = 0; i < 10; i++) {
        printf("%d:main thread\n", i);
    }
    pthread_exit(NULL);

}
