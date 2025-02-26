#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#define ITERATIONS 100000000

typedef struct threadNode {
    long long start;
    long long end;
    double thread_sum;
} threadNode;

void* thread_body(void *param) {
    threadNode* node = (threadNode*)param;
    double sum = 0.0;
    for (long long i = node->start; i < node->end; ++i) {
        double temp = 1.0 / (double) (2 * (double)i + 1.0);
        if (i % 2 == 0) {
            sum += temp;
        } else {
            sum -= temp;
        }
    }
    node->thread_sum = sum;
    return NULL;
}

int main(int argc, char** argv) {
    int thread_count;
    scanf("%d", &thread_count);
    long long batch_size = ITERATIONS / thread_count;

    int remainder = ITERATIONS % thread_count;

    pthread_t* threads = malloc(thread_count * sizeof(pthread_t));
    if (threads == NULL) {
        perror("Error allocating memory");
        exit(EXIT_FAILURE);
    }
    threadNode* nodes = malloc(thread_count * sizeof(threadNode));
    if (nodes == NULL) {
        perror("Error allocating memory");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < thread_count; ++i) {
        if (i == 0) {
            nodes[i].start = 0;
        } else {
            nodes[i].start = nodes[i-1].end;
        }
        if (remainder > 0) {
            nodes[i].end = nodes[i].start + batch_size + 1;
            remainder--;
        } else {
            nodes[i].end = nodes[i].start + batch_size;
        }
        if (0 != pthread_create(&threads[i], NULL, thread_body, (void*) (nodes + i))) {
            perror("Error creating thread");
            exit(EXIT_FAILURE);
        }
        printf("%d %lld %lld %lld\n", i, nodes[i].start, nodes[i].end, nodes[i].end - nodes[i].start);
    }

    double res = 0.0;
    for (int i = 0; i < thread_count; ++i) {
        pthread_join(threads[i], NULL);
        res += nodes[i].thread_sum;
    }

    printf("%.15f\n", res * 4.0);

    free(threads);
    free(nodes);
    return 0;
}