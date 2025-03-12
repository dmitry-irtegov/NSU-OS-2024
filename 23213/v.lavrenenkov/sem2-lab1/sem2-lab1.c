#include <pthread.h>
#include <stdio.h>
#include <string.h>


void * print10(void* val) {
    for(int i = 0; i < 10; i++) {
        printf("Some text. %d \n", *(int *)val);
    }
    return 0;
}

int main() {
    pthread_t th;
    int childVal = 1;
    int parentVal = -10;
    int err = 0;
    if ((err = pthread_create(&th, NULL, print10, &childVal)) != 0) {
        fprintf(stderr, "Couldn`t open thread: %s \n", strerror(err));
        return 1;
    }
    print10(&parentVal);
    pthread_join(th, NULL);
    return 0;
}
