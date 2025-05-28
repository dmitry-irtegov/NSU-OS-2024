#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void* child_thread(void* arg) {
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < 10; i++) {
        printf("CHILD\n");    
        pthread_cond_signal(&cond);
        pthread_cond_wait(&cond, &mutex);
    }
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);

    return NULL;
}

int main(int argc, char *argv[]) {
    pthread_t thread;
    int code;
    if ((code = pthread_create(&thread, NULL, child_thread, NULL)) != 0) {
     	char buf[256];
        strerror_r(code, buf, sizeof(buf));
        fprintf(stderr, "%s: creating: %s\n", argv[0], buf);
        exit(1);
    }

    pthread_mutex_lock(&mutex);
    for (int i = 0; i < 10; i++) {
        printf("PARENT\n");    
        pthread_cond_signal(&cond);
        pthread_cond_wait(&cond, &mutex);
    }
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
    
    pthread_join(thread, NULL);
    return 0;
}

