#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

void* thread_body(void* param) {
    for(int i=0; i<10; i++) { 
        printf("Child\n");
    }
	return NULL;
}

int main(int argc, char *argv[]) {
    pthread_t thread;
    int code;

    if ((code = pthread_create(&thread, NULL, thread_body, NULL)) !=0) {
        char buf[256];
        strerror_r(code, buf, sizeof buf);
        fprintf(stderr, "%s: creating thread: %s\n", argv[0], buf);
        exit(-1);
    }
    
    for(int i=0; i<10; i++) {
        printf("Parent\n");
    }

    pthread_exit(NULL);
}
