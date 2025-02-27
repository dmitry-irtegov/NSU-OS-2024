#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <assert.h>

pthread_mutex_t mutex;
pthread_cond_t cond;
int turn = 0;

void *print_messages(void *arg) {
    for (int i = 0; i < 10; i++) {
        assert(pthread_mutex_lock(&mutex) == 0);
        while (!turn) {
            pthread_cond_wait(&cond, &mutex);
        }
        printf("[Thread] Message %d\n", i + 1);
        turn = 0;
        pthread_cond_signal(&cond);
        assert(pthread_mutex_unlock(&mutex) == 0);
        
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    pthread_t thread;
    int code;
    pthread_mutexattr_t attr;

    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);

    if((code = pthread_mutex_init(&mutex, NULL)) != 0){
        fprintf(stderr, "Failed to init mutex: %s\n", strerror(code));
        exit(1);
    }

    pthread_cond_init(&cond, NULL);

    code = pthread_create(&thread, NULL, print_messages, NULL);
    if (code != 0) {
        char buf[256];
        strerror_r(code, buf, sizeof(buf));
        fprintf(stderr, "%s: creating thread: %s\n", argv[0], buf);
        exit(1);
    }

    
    for (int i = 0; i < 10; i++) {
        assert(pthread_mutex_lock(&mutex) == 0);
        while (turn) {
            pthread_cond_wait(&cond, &mutex);
        }
        printf("[Main] Message %d\n", i + 1);
        turn = 1;
        pthread_cond_signal(&cond);
        assert(pthread_mutex_unlock(&mutex) == 0); 
    }
    
    pthread_join(thread, NULL);
    pthread_mutex_destroy(&mutex);
    pthread_mutexattr_destroy(&attr);
    pthread_cond_destroy(&cond);
    
    pthread_exit(NULL);
}
