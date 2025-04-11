#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void *thread_function(void *arg) {
    char **strings = (char **)arg;

    for (int i = 0; strings[i] != NULL; i++) {
        printf("%s\n", strings[i]);
    }

    pthread_exit(NULL);
}

int main() {
    pthread_t threads[4];
    int errCode;

    char *strings1[] = {"Thread 1: hi", "Thread 1: im", "Thread 1: thread1", NULL};
    char *strings2[] = {"Thread 2: hi", "Thread 2: im", "Thread 2: thread2", NULL};
    char *strings3[] = {"Thread 3: privet", "Thread 3: ya", "Thread 3: tri", NULL};
    char *strings4[] = {"Thread 4: bye", NULL};

    char **strings_arr[4] = {strings1, strings2, strings3, strings4};

    for (int i = 0; i < 4; i++) {
        if ((errCode = pthread_create(&threads[i], NULL, thread_function, strings_arr[i])) != 0) {
            char* buf = strerror(errCode);
            fprintf(stderr, "Failed to create thread: %d, %s\n", i + 1, buf);
            exit(1);
        }
    }

    for (int i = 0; i < 4; i++) {
        if ((errCode = pthread_join(threads[i], NULL)) != 0) {
            char* buf = strerror(errCode);
            fprintf(stderr, "Failed to join thread: %d, %s\n", i + 1, buf);
            exit(1);
        }
    }

    exit(0);
}
