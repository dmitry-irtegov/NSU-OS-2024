#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

void* print_text(void* arg) {
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

    while (1) {
        printf("Child thread is running...\n");
        sleep(1);
    }

    return NULL;
}

int main() {
    pthread_t thread;

    if (pthread_create(&thread, NULL, print_text, NULL) != 0) {
        perror("pthread_create");
        return EXIT_FAILURE;
    }

    sleep(2);

    if (pthread_cancel(thread) != 0) {
        perror("pthread_cancel");
        return EXIT_FAILURE;
    }

    if (pthread_join(thread, NULL) != 0) {
        perror("pthread_join");
        return EXIT_FAILURE;
    }

    printf("Child thread has been canceled.\n");

    return EXIT_SUCCESS;
}
