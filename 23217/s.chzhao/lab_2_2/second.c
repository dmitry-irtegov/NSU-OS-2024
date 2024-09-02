#include <stdio.h>
#include <pthread.h>

void* child_thread_func(void* arg) {
    for (int i = 1; i <= 10; ++i) {
        printf("Child thread: line %d\n", i);
    }
    return NULL;
}

int main() {
    pthread_t child_tid;

    pthread_create(&child_tid, NULL, child_thread_func, NULL);
    pthread_join(child_tid, NULL);
    
    for (int i = 1; i <= 10; ++i) {
        printf("Parent thread: line %d\n", i);
    }
    return 0;
}