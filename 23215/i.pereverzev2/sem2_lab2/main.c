#define _REENTRANT
#include <pthread.h>
#include <thread.h>
#include <stdio.h>
#include <string.h>

void* print_text(void* arg)
{
    for(int i = 0; i < 10; i++) {
        printf("%d, created thread, line\n", i);
    }
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
    pthread_join(th, NULL);
    for(int i = 0; i < 10; i++) {
        printf("%d, main thread\n", i);
    }
    return 0;
}
