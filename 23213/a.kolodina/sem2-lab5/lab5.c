#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void death_message(void* args) {
    printf("Death\n");
}

void* print_strings(void* args) {
    pthread_cleanup_push(death_message, NULL);
    while(1) {
        printf("Child\n");
        sleep(1);
    }
    pthread_cleanup_pop(0);
    return NULL;
}


int main(void) {
    pthread_t thread;
    int code;
    code = pthread_create(&thread, NULL, print_strings, NULL);
    if (code != 0) {
        char buf[256];
        strerror_r(code, buf, sizeof(buf));
        fprintf(stderr, "%s\n", buf);
        exit(1);
    }
    sleep(2);
    printf("Trying to cancel\n");
    code = pthread_cancel(thread);
    if (code != 0) {
        char buf[256];
        strerror_r(code, buf, sizeof(buf));
        fprintf(stderr, "%s\n", buf);
        exit(1);
    }
    pthread_join(thread, NULL);
    printf("Cancelled\n");
    return 0;
}
