#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

pthread_mutex_t mutex;
pthread_cond_t cond;
int lock;

void mutex_init(pthread_mutex_t* curr_mutex) 
{
    pthread_mutexattr_t attrs;
    
    pthread_mutexattr_init(&attrs); 
    pthread_mutexattr_settype(&attrs, PTHREAD_MUTEX_ERRORCHECK);

    int error;
    if ((error = pthread_mutex_init(curr_mutex, &attrs)) != 0) {
        fprintf(stderr, "Failed to init mutex: %s\n", strerror(error));
        exit(-1);
    }
}

void mutex_lock(pthread_mutex_t* curr_mutex) 
{
    int error;
    if ((error = pthread_mutex_lock(curr_mutex)) != 0) 
    {
        fprintf(stderr, "Failed to lock mutex: %s\n", strerror(error));
        exit(-1);
    }
}

void mutex_unlock(pthread_mutex_t* curr_mutex) {
    int error;
    if ((error = pthread_mutex_unlock(curr_mutex)) != 0) {
        fprintf(stderr, "Failed to unlock mutex: %s\n", strerror(error));
        exit(-1);
    }
}

void* thread_body(void* param) {    
    for (int i = 0; i < 10; i++) {
        mutex_lock(&mutex);
        
        while (lock) {
            pthread_cond_wait(&cond, &mutex);
        }
        
        printf("Child\n");   
        
        lock = 1;
        
        pthread_cond_signal(&cond);
        
        mutex_unlock(&mutex);
    }
    return NULL;
}

int main (int argc, char* argv[]) {
    pthread_t thread;
    mutex_init(&mutex);
    pthread_cond_init(&cond, NULL);
    int code;
    lock = 1;
    
    if ((code = pthread_create(&thread, NULL, thread_body, NULL)) != 0) {
        fprintf(stderr, "%s: creating thread: %s\n", argv[0], strerror(code));
        exit(1);
    }    
    
    for (int i = 0; i < 10; i++) {
        mutex_lock(&mutex);
        
        while(!lock) {
            pthread_cond_wait(&cond, &mutex);
        }

        printf("Parent\n");
        
        lock = 0;

        pthread_cond_signal(&cond);

        mutex_unlock(&mutex);
    }   

    pthread_join(thread, NULL);

    return 0;
}
