#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#define handle_error(en, msg) { errno = en; perror(msg); exit(EXIT_FAILURE); }

void print_all_lines(const char* label) {
    for (int i = 0; i < 10; i++) {
        printf("%s: number %d\n", label, i);
    }
}

void* print_lines(void* arg) {
    print_all_lines("Thread");
    return NULL;
}

int main() {
    pthread_t thread;
    int res;

    res = pthread_create(&thread, NULL, print_lines, NULL);
    if (res != 0) {
        handle_error(res, "pthread_create fail");
    }

    res = pthread_join(thread, NULL); 
    if (res != 0) {
        handle_error(res, "pthread_join"); 
    }

    print_all_lines("Parent thread");

    pthread_exit(NULL);
}
