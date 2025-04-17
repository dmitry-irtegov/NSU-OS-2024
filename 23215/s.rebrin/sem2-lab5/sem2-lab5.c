#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

void clean_up(void* param) {
    printf("\nnoooooooooooooooooooooo\n");
}

void* thread_body(void* param) {
    pthread_cleanup_push(clean_up, NULL);
    for (int i = 0; i < 300; i++) {
        printf("pu ");
        usleep(50000);
        fflush(stdout);
    }
    pthread_cleanup_pop(1); 
    return NULL;
}

int main(int argc, char* argv[]) {
    pthread_t thread;
    int code;

    code = pthread_create(&thread, NULL, thread_body, NULL);
    if (code != 0) {
        char buf[256];
        strerror_r(code, buf, sizeof(buf));
        fprintf(stderr, "%s: creating thread: %s\n", argv[0], buf);
        exit(1);
    }
    sleep(2);
    pthread_cancel(thread);
    pthread_join(thread, NULL); 
    pthread_exit(NULL);
}