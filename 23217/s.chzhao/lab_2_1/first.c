#include <pthread.h>
#include <stdio.h>

void* thread_func(void* arg) {
    int x = (int) pthread_self();
    printf("Child thread ID: %d\n", x);
    for (int i = 1; i <= 10; i++) {
        printf("Child thread - line %d\n", i);
    }
    return NULL;
}

int main() {
    pthread_t tid;
    pthread_t tid1;

    pthread_create(&tid, NULL, thread_func, NULL);
    pthread_create(&tid1, NULL, thread_func, NULL);

    for (int i = 1; i <= 10; i++) {
        printf("Main thread - line %d\n", i);
    }
    pthread_exit(NULL);
    return 0;
}
