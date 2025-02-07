#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <assert.h>

pthread_mutex_t mutex_main, mutex_thread, mutex_stdout;

volatile int flag = 0;

void* thread_body(void* param) {
    int status;
    status = pthread_mutex_lock(&mutex_stdout);
    assert(status == 0);
    flag = 1;
    
    for (int i = 0; i < 10; i++) {
        status = pthread_mutex_lock(&mutex_main);
        assert(status == 0);
        status = pthread_mutex_unlock(&mutex_stdout);
        assert(status == 0);
        status = pthread_mutex_lock(&mutex_thread);
        assert(status == 0);
        
        printf("Child\n");
        
        status = pthread_mutex_unlock(&mutex_main);
        assert(status == 0);
        status = pthread_mutex_lock(&mutex_stdout);
        assert(status == 0);
        status = pthread_mutex_unlock(&mutex_thread);
        assert(status == 0);
    }
    
    pthread_mutex_unlock(&mutex_stdout);
    
    return NULL;
}

int main (int argc, char* argv[]) {
    pthread_t thread;
    pthread_mutexattr_t attrs;
    pthread_mutexattr_init(&attrs);
    pthread_mutexattr_settype(&attrs, PTHREAD_MUTEX_ERRORCHECK);

    pthread_mutex_init(&mutex_main, &attrs);
    pthread_mutex_init(&mutex_stdout, &attrs);
    pthread_mutex_init(&mutex_thread, &attrs);
    
    int code;

    code = pthread_mutex_lock(&mutex_main);
    assert(code == 0);
    code = pthread_mutex_lock(&mutex_thread);
    assert(code == 0);
    
    if ((code = pthread_create(&thread, NULL, thread_body, NULL)) != 0) {
        fprintf(stderr, "%s: creating thread: %s\n", argv[0], strerror(code));
        exit(1);
    }    

    while (flag == 0) {
        sched_yield();
    }
    
    for (int i = 0; i < 10; i++) {
        printf("Parent\n");
        
        code = pthread_mutex_unlock(&mutex_main);
        assert(code == 0);
        code = pthread_mutex_lock(&mutex_stdout);
        assert(code == 0);
        code = pthread_mutex_unlock(&mutex_thread);
        assert(code == 0);
        code = pthread_mutex_lock(&mutex_main);
        assert(code == 0);
        code = pthread_mutex_unlock(&mutex_stdout);
        assert(code == 0);
        code = pthread_mutex_lock(&mutex_thread);
        assert(code == 0);
    }   

    code = pthread_mutex_unlock(&mutex_main);
    assert(code == 0);
    code = pthread_mutex_unlock(&mutex_thread);
    assert(code == 0);

    pthread_join(thread, NULL);

    pthread_mutex_destroy(&mutex_main);
    pthread_mutex_destroy(&mutex_stdout);
    pthread_mutex_destroy(&mutex_thread);

    return 0;    
}
