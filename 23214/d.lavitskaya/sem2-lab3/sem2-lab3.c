#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define THREAD_COUNT 4

void *print_messages(void *arg) {
    char **messages = (char **)arg; 
    for (int i = 0; messages[i] != NULL; i++) {
        printf("[Thread %lu] %s\n", pthread_self(), messages[i]);
        
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    pthread_t threads[THREAD_COUNT];
    int code;
    char* buf;

    char *messages1[] = {"Hello from thread 1", "Message A1", "Message B1", NULL};
    char *messages2[] = {"Hello from thread 2", "Message A2", "Message B2", NULL};
    char *messages3[] = {"Hello from thread 3", "Message A3", "Message B3", NULL};
    char *messages4[] = {"Hello from thread 4", "Message A4", "Message B4", NULL};

    char **messages[THREAD_COUNT] = {messages1, messages2, messages3, messages4};

    for (int i = 0; i < THREAD_COUNT; i++) {
        code = pthread_create(&threads[i], NULL, print_messages, messages[i]);
        if (code != 0) {
            buf = strerror(code);
            fprintf(stderr, "%s: creating thread %d: %s\n", argv[0], i, buf);
            exit(1);
        }
    }

    for (int i = 0; i < THREAD_COUNT; i++) {
        code = pthread_join(threads[i], NULL);
        if(code != 0){
            buf = strerror(code);
            fprintf(stderr, "%s: joining thread %d: %s\n", argv[0], i, buf);
            exit(1);
        }
    }

    printf("All threads have finished.\n");
    return 0;
}
