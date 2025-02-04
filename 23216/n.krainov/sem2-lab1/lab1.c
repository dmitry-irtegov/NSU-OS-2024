#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void print_text(char* name) {
    printf("text1 %s\n", name);
    printf("text2 %s\n", name);
    printf("text3 %s\n", name);
    printf("text4 %s\n", name);
    printf("text5 %s\n", name);
    printf("text6 %s\n", name);
    printf("text7 %s\n", name);
    printf("text8 %s\n", name);
    printf("text9 %s\n", name);
    printf("text10 %s\n", name);
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