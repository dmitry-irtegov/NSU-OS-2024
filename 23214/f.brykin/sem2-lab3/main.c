#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

void* thread_function(void* arg) {
    const char** lines = (const char**)arg;
    for (int i = 0; lines[i] != NULL; i++) {
        printf("%s\n", lines[i]);
    }
    return NULL;
}

int main() {
    const char* lines1[] = {"Thread 1: Line 1", "Thread 1: Line 2", "Thread 1: Line 3", NULL};
    const char* lines2[] = {"Thread 2: Line 1", "Thread 2: Line 2", NULL};
    const char* lines3[] = {"Thread 3: Line 1", "Thread 3: Line 2", "Thread 3: Line 3", "Thread 3: Line 4", NULL};
    const char* lines4[] = {"Thread 4: Line 1", NULL};
    pthread_t thread1, thread2, thread3, thread4;
    int result;
    result = pthread_create(&thread1, NULL, thread_function, lines1);
    if (result != 0) {
        perror("Error creating thread 1");
        exit(EXIT_FAILURE);
    }

    result = pthread_create(&thread2, NULL, thread_function, lines2);
    if (result != 0) {
        perror("Error creating thread 2");
        exit(EXIT_FAILURE);
    }

    result = pthread_create(&thread3, NULL, thread_function, lines3);
    if (result != 0) {
        perror("Error creating thread 3");
        exit(EXIT_FAILURE);
    }

    result = pthread_create(&thread4, NULL, thread_function, lines4);
    if (result != 0) {
        perror("Error creating thread 4");
        exit(EXIT_FAILURE);
    }

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);
    pthread_join(thread4, NULL);
    return 0;
}
