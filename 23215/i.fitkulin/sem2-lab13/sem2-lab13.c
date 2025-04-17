#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int flag = 0;

void* thread_body() {
    for (int i = 0; i < 10; i++) {
        pthread_mutex_lock(&mutex);
        while (!flag) {
            pthread_cond_wait(&cond, &mutex);
        }
        printf("child\n");
        flag = 0;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main() {
    pthread_t thread;
    int code = pthread_create(&thread, NULL, thread_body, NULL);
    if (code != 0) {
        char buf[256];
        strerror_r(code, buf, sizeof(buf));
        fprintf(stderr, "creating thread: %s\n", buf);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < 10; i++) {
        pthread_mutex_lock(&mutex);
        while (flag) {
            pthread_cond_wait(&cond, &mutex);
        }
        printf("parent\n");
        flag = 1;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }

    pthread_join(thread, NULL);
    pthread_exit(NULL);
}
