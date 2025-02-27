#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

void* pthread_func(void* arg) {
    int i = 1;
    while (1) {
        printf("Text %d\n", i);
        i++;
        usleep(200000);
    }
    return NULL;
}

int main() {
    pthread_t thread;
    
    if (pthread_create(&thread, NULL, pthread_func, NULL) != 0) {
        perror("Pthread creating failure");
        return 1;
    }

    sleep(2);

    pthread_cancel(thread);
    pthread_join(thread, NULL);

    return 0;
}
