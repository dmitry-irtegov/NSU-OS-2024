#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define NUM_STEPS 200000000

int num_threads;

typedef struct Thread_data {
    int thread_id;
    double part_sum;
} Thread_data;

void* compute_pi(void *arg) {
    Thread_data* currThread = (Thread_data*)arg;

    double currPartSum = 0.0;

    for (int i = currThread->thread_id; i < NUM_STEPS; i += num_threads) {
        double sign  = (i % 2 == 0) ? 1.0 : -1.0; 
        double denom = 2.0 * i + 1.0;
        currPartSum += sign / denom;
    }
    currThread->part_sum = currPartSum;

    pthread_exit(NULL);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <num_threads>\n", argv[0]);
        exit(1);
    }

    int errCode;
    num_threads = atoi(argv[1]);
    if (num_threads <= 0) {
        fprintf(stderr, "Thread number must be positive!\n");
        exit(1);
    }

    pthread_t* threads = (pthread_t*)malloc(num_threads * sizeof(pthread_t));
    if (threads == NULL) {
        perror("Malloc error on threads");
        exit(1);
    }
    Thread_data* threadsData = (Thread_data*)malloc(num_threads * sizeof(Thread_data));
    if (threadsData == NULL) {
        perror("Malloc error on threadsData");
        exit(1);
    }

    for (int i = 0; i < num_threads; i++) {
	threadsData[i].thread_id = i;
	threadsData[i].part_sum = 0.0;
        if ((errCode = pthread_create(&threads[i], NULL, compute_pi, (void*)&threadsData[i])) != 0) {
            char* buf = strerror(errCode);
            fprintf(stderr, "Failed to create thread: %s\n", buf);
            exit(1);
        }
    }

    double pi = 0.0;
    for (int i = 0; i < num_threads; i++) {
        if ((errCode = pthread_join(threads[i], NULL)) != 0) {
            free(threads);
            char* buf = strerror(errCode);
            fprintf(stderr, "Failed to wait thread: %s\n", buf);
            exit(1);
        }

        pi += threadsData[i].part_sum;
    }

    pi *= 4.0;

    printf("Thread number: %d\n", num_threads);
    printf("Pi: %.15f\n", pi);

    free(threads);
    free(threadsData);

    exit(0);
}
