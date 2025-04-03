#include <stdio.h>
#include <pthread.h>

#define NUM_LINES 10

pthread_mutex_t mutex;
pthread_cond_t cond;
int turn = 0; // 0 - parent, 1 - child

void* child_thread(void* arg) {
    for (int i = 0; i < NUM_LINES; i++) {
        pthread_mutex_lock(&mutex);
        while (turn != 1) {
            pthread_cond_wait(&cond, &mutex);
        }
        printf("Child thread - line %d\n", i + 1);
        turn = 0;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main() {
    pthread_t thread;
    pthread_mutexattr_t attr;
    
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    pthread_mutex_init(&mutex, &attr);
    pthread_cond_init(&cond, NULL);
    
    pthread_create(&thread, NULL, child_thread, NULL);
    
    for (int i = 0; i < NUM_LINES; i++) {
        pthread_mutex_lock(&mutex);
        while (turn != 0) {
            pthread_cond_wait(&cond, &mutex);
        }
        printf("Parent thread - line %d\n", i + 1);
        turn = 1;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }
    
    pthread_join(thread, NULL);
    
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
    pthread_mutexattr_destroy(&attr);
    
    return 0;
}
