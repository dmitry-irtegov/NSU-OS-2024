#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void print_text() {
    puts("text1");
    puts("text2");
    puts("text3");
    puts("text4");
    puts("text5");
    puts("text6");
    puts("text7");
    puts("text8");
    puts("text9");
    puts("text10");
}

void* thread_func(void* param) {
    print_text();
    pthread_exit(NULL);
}

int main() {
    pthread_t thread;
    int code = 0;
    if ((code = pthread_create(&thread, NULL, thread_func, NULL)) != 0) {
        fprintf(stderr, "pthread_create error: %d", code);
        exit(EXIT_FAILURE);
    }
    print_text();

    pthread_exit(NULL);
}