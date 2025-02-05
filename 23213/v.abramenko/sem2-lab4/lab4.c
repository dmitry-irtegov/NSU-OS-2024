#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

void* thread_body(void* param) {
    while(1) { 
        printf("Child\n");
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    pthread_t thread;
    int code;

    if ((code = pthread_create(&thread, NULL, thread_body, NULL)) !=0) {
        char buf[256];
        strerror_r(code, buf, sizeof(buf));
        fprintf(stderr, "%s: creating thread: %s\n", argv[0], buf);
        exit(-1);
    }

    sleep(2);

    printf("Trying to cancel\n");
    
    if ((code=pthread_cancel(thread))!=0) {
        char buf[256];
        strerror_r(code, buf, sizeof(buf));
        fprintf(stderr, "%s: cancelling thread: %s\n", argv[0], buf);
        exit(-1);
    }

    pthread_join(thread, NULL);
    printf("Cancelled\n");
    
    pthread_exit(NULL);
}
