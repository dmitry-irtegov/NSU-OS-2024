#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void *thread_function(void *arg) {
        for (int i = 0; i < 10; i++) {
                printf("Child thread: %d  :-)\n", i + 1);
        }
        pthread_exit(NULL);
}

int main() {
        pthread_t thread;
        int errCode;

        if ((errCode = pthread_create(&thread, NULL, thread_function, NULL)) != 0) {
                char* buf = strerror(errCode);
                fprintf(stderr, "Failed to create thread: %s\n", buf);
                exit(1);
        }

        pthread_join(thread, NULL);

        for (int i = 0; i < 10; i++) {
                printf("Parent thread: %d  :-)\n", i + 1);
        }

        pthread_exit(NULL);
}
