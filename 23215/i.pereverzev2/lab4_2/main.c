#define _REENTRANT
#include <pthread.h>
#include <thread.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

void* print_text(void* arg)
{
    int i = 0;
    while(1) {
        printf("%d, created thread, line\n", i);
        i++;
    }
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
    sleep(2000);
    pthread_cancel(th);
    return 0;
}
