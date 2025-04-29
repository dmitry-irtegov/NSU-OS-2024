#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_STEPS 200000000

typedef struct {
    int threadId;
    int threadsNum;
    double piPart;
} ThreadData;

void* piPartCalculating(void* data);

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: %s <number_of_threads>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int threadsNum = atoi(argv[1]);
    if (threadsNum <= 0) {
        printf("Number of threads must be positive.\n");
        exit(EXIT_FAILURE);
    }

    pthread_t threads[threadsNum];
    ThreadData threadData[threadsNum];
    double pi = 0.0;

    for (int i = 0; i < threadsNum; i++) {
        threadData[i].threadId = i;
        threadData[i].threadsNum = threadsNum;
        threadData[i].piPart = 0.0;

        if (pthread_create(&threads[i], NULL, piPartCalculating, &threadData[i]) != 0) {
            perror("Failed to create thread");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < threadsNum; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("Failed to join thread");
            exit(EXIT_FAILURE);
        }
        pi += threadData[i].piPart;
    }

    pi = pi * 4.0;
    printf("pi done - %.15g \n", pi);

    exit(EXIT_SUCCESS);
}

void* piPartCalculating(void* data) {
    ThreadData* threadData = (ThreadData*)data;
    double sum = 0.0;

    for (int i = threadData->threadId; i < NUM_STEPS; i += threadData->threadsNum) {
        sum += 1.0 / (i * 4.0 + 1.0);
        sum -= 1.0 / (i * 4.0 + 3.0);
    }

    threadData->piPart = sum;

    pthread_exit(NULL);
}
