#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

void* thread_func(void* param) {
    //printf("text\n");
    //sleep(1);
    //printf("text2\n");
    //sleep(10);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    for(;;) {}
    return NULL;
}

int main() {
    pthread_t thread;

    int code = pthread_create(&thread, NULL, thread_func, NULL);
    if (code != 0) {
        perror("creating thread");
        exit(1);
    }

    sleep(2);
    pthread_cancel(thread);
    printf("cancel\n");

    pthread_join(thread, NULL);
}
