#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

void print_cancelled(void *arg) {
    printf("Cancelled\n");
}

void *thread_func(void *arg) {
    pthread_cleanup_push(print_cancelled, NULL);
    for (int i = 0;; i = (i + 1) % 1000000) {
        printf("%d\n", i);
    }
    pthread_cleanup_pop(0);
}

int main(int argc, char *argv[]) {
    pthread_t tid;

    int errcode = pthread_create(&tid, NULL, thread_func, NULL);
    if (errcode) {
        fprintf(stderr, "Failed to create thread: %s\n", strerror(errcode));
        return 1;
    }
    
    sleep(2);

    errcode = pthread_cancel(tid);
    if (errcode) {
        fprintf(stderr, "Failed to cancel thread: %s\n", strerror(errcode));
        return 1;
    }

    errcode = pthread_join(tid, NULL);
    if (errcode) {
        fprintf(stderr, "Oh no! Attempt to join thread failed: %s\n", strerror(errcode));
    } else {
        printf("Joined thread succsesfully\n");
    }

    return 0;
}
