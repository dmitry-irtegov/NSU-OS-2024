#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void print_text(char* name) {
    for (int i = 1; i <= 10; i++) {
        printf("text%d %s\n", i, name);
    }
}

void* thread_func(void* param) {
    print_text((char*) param);
    pthread_exit(NULL);
}

void my_perror(char* text, int code) {
    fprintf(stderr, "%s: %s \n", text, strerror(code));
}

int main() {
    pthread_t thread;
    int code = 0;
    char* name = "child";
    if ((code = pthread_create(&thread, NULL, thread_func, name)) != 0) {
        my_perror("pthread_create error", code);
        exit(EXIT_FAILURE);
    }
    print_text("parent");

    pthread_exit(NULL);
}