#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *messages1[] = {"Thread 1: line 1", NULL};
static char *messages2[] = {"Thread 2: line 1", "Thread 2: line 2", NULL};
static char *messages3[] = {"Thread 3: line 1", "Thread 3: line 2", "Thread 3: line 3", NULL};
static char *messages4[] = {"Thread 4: line 1", "Thread 4: line 2", "Thread 4: line 3", "Thread 4: line 4", NULL};

static char **thread_data[4] = {messages1, messages2, messages3, messages4};

void *child_thread(void *arg) {
    char **messages = (char **)arg;
    for (int i = 0; messages[i] != NULL; i++) {
        printf("%s\n", messages[i]);
    }
    return NULL;
}

int main() {
    pthread_t threads[4];
    int errCode;
    for (int i = 0; i < 4; i++) {
        if ((errCode = pthread_create(&threads[i], NULL, child_thread, thread_data[i])) != 0) {
            fprintf(stderr, "ERROR: Thread creation failed %d: %s\n", i + 1, strerror(errCode));
            exit(-1);;
        }
    }
    for (int i = 0; i < 4; i++) {
        if ((errCode = pthread_join(threads[i], NULL)) != 0) {
            fprintf(stderr, "ERROR: Thread join failed %d: %s\n", i + 1, strerror(errCode));
            exit(-1);;
        }
    }
    exit(0);
}
