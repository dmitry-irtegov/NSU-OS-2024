#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_THREADS 4

void* print_sequence(void* arg) {
    char** sequence = (char**)arg;
    for (int i = 0; sequence[i] != NULL; i++) {
        usleep(10000);
        printf("%s\n", sequence[i]);
    }
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    
    char* sequences[NUM_THREADS][5] = {
        {"Thread 1: Line 1", "Thread 1: Line 2", "Thread 1: Line 3", NULL},
        {"Thread 2: Line 1", "Thread 2: Line 2", "Thread 2: Line 3", NULL},
        {"Thread 3: Line 1", "Thread 3: Line 2", "Thread 3: Line 3", NULL},
        {"Thread 4: Line 1", "Thread 4: Line 2", "Thread 4: Line 3", NULL}
    };
    
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, print_sequence, sequences[i]);
    }
    
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    return 0;
}
