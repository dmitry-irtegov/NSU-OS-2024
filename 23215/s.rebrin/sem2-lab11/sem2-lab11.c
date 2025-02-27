#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int turn = 0; // 0 - main 

void* thread_body(void* param) {
    for (int i = 0; i < 10; i++) {
        pthread_mutex_lock(&mutex);
        while (turn != 1) {
            pthread_mutex_unlock(&mutex);
            pthread_mutex_lock(&mutex);
        }
        printf("left\n");
        turn = 0;
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    pthread_t thread;
    int code;

    pthread_mutex_init(&mutex, NULL);

    code = pthread_create(&thread, NULL, thread_body, NULL);
    if (code) {
        char buf[256];
        strerror_r(code, buf, sizeof(buf));
        fprintf(stderr, "%s: creating thread: %s\n", argv[0], buf);
        exit(1);
    }
    else {
        for (int i = 0; i < 10; i++) {
            pthread_mutex_lock(&mutex);
            while (turn != 0) {
                pthread_mutex_unlock(&mutex);
                pthread_mutex_lock(&mutex);
            }
            printf("right\n");
            turn = 1;
            pthread_mutex_unlock(&mutex);
        }
    }

    pthread_join(thread, NULL);
    pthread_mutex_destroy(&mutex);
    pthread_exit(NULL);
}