#define _REENTRANT
#include <pthread.h>
#include <thread.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>


void last_words(void* arg)
{
    printf("this thread is going to terminate\n");
}


void* print_text(void* arg)
{
    pthread_cleanup_push(last_words, NULL);
    int i = 0;
    while(1) {
        printf("%d, created thread, line\n", i);
        i++;
    }
    pthread_cleanup_pop(1);
    return NULL;
}

int main()
{
    pthread_t th;
    int code = pthread_create(&th, NULL, print_text, NULL);
    if(code){
        char buf[256];
        strerror_r(code, buf, sizeof(buf));
        fprintf(stderr, "unable to create thread: %s", buf);
        return 1;
    }
    sleep(2);
    code = pthread_cancel(th);
    if(code){
        char buf[256];
        strerror_r(code, buf, sizeof(buf));
        fprintf(stderr, "unable to cancel thread: %s", buf);
        return 1;
    }
    pthread_join(th, NULL);
    return 0;
}