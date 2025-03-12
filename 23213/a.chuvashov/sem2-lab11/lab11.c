#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

pthread_mutex_t mutex_main, mutex_thread, mutex_stdout;

void* thread_body(void* param) {
    pthread_mutex_lock(&mutex_stdout);
    
    for (int i = 0; i < 10; i++) {
        pthread_mutex_lock(&mutex_main);
        pthread_mutex_unlock(&mutex_stdout);
        pthread_mutex_lock(&mutex_thread);
        
        printf("Child\n");
        
        pthread_mutex_unlock(&mutex_main);
        pthread_mutex_lock(&mutex_stdout);
        pthread_mutex_unlock(&mutex_thread);
    }
    
    pthread_mutex_unlock(&mutex_stdout);
    
    return NULL;
}

int main (int argc, char* argv[]) {
    pthread_t thread;
    pthread_mutexattr_t attrs;
    int error;

    pthread_mutexattr_init(&attrs);
    pthread_mutexattr_settype(&attrs, PTHREAD_MUTEX_ERRORCHECK);

    if ((error = pthread_mutex_init(&mutex_main, &attrs)) != 0) {
        fprintf(stderr, "Failed to init mutex: %s\n", strerror(error));
        exit(-1);
    }
    
    if ((error = pthread_mutex_init(&mutex_stdout, &attrs)) != 0) {
        fprintf(stderr, "Failed to init mutex: %s\n", strerror(error));
        exit(-1);
    }
    
    if ((error = pthread_mutex_init(&mutex_thread, &attrs)) != 0) {
        fprintf(stderr, "Failed to init mutex: %s\n", strerror(error));
        exit(-1);
    }
    
    int code;

    pthread_mutex_lock(&mutex_main);
    pthread_mutex_lock(&mutex_thread);
    
    if ((code = pthread_create(&thread, NULL, thread_body, NULL)) != 0) {
        fprintf(stderr, "%s: creating thread: %s\n", argv[0], strerror(code));
        exit(1);
    }    
    
    for (int i = 0; i < 10; i++) {
        printf("Parent\n");
        
        pthread_mutex_unlock(&mutex_main);
        pthread_mutex_lock(&mutex_stdout);
        pthread_mutex_unlock(&mutex_thread);
        pthread_mutex_lock(&mutex_main);
        pthread_mutex_unlock(&mutex_stdout);
        pthread_mutex_lock(&mutex_thread);
    }   

    pthread_mutex_unlock(&mutex_main);
    pthread_mutex_unlock(&mutex_thread);

    pthread_join(thread, NULL);

    if ((code = pthread_mutex_destroy(&mutex_main)) != 0) {
        fprintf(stderr, "Failed to destroy mutex: %s\n", strerror(code));
        exit(-1);
    }
    
    if ((code = pthread_mutex_destroy(&mutex_thread)) != 0) {
        fprintf(stderr, "Failed to destroy mutex: %s\n", strerror(code));
        exit(-1);
    }
    
    if ((code = pthread_mutex_destroy(&mutex_stdout)) != 0) {
        fprintf(stderr, "Failed to destroy mutex: %s\n", strerror(code));
        exit(-1);
    }

    return 0;    
}
