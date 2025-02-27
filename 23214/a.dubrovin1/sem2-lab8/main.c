#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

long long accuracy = 1000000;

typedef struct {    
    long long start;
    long long end;
    double partial_sum;
} thread_args;

void* thread_body(void* arg) {
    thread_args* data = (thread_args*)arg;
    double sum = 0.0;
    for (long long i = data->start; i < data->end; i++) {
        sum += 1.0 / ((double)i * 4.0 + 1.0);
        sum -= 1.0 / ((double)i * 4.0 + 3.0);
    }
    data->partial_sum = sum * 4;
    return NULL;
}

int main(int argc, char** argv) {
    
    if (argc < 2) {
        fprintf(stderr, "Oops, you haven't specified threads number\n");
        exit(1);
    }

    long long threads_number = atoll(argv[1]);
    if (threads_number < 1) {
        fprintf(stderr, "Oops, you have inappropriate number of threads: %lld\n", threads_number);
        exit(1);
    }

    if (threads_number > accuracy) {
        threads_number = accuracy;
    }

    pthread_t* threads = malloc(threads_number * sizeof(pthread_t));
    if (threads == NULL) {
        perror("Oops, malloc failed");
        exit(1);
    }

    thread_args* args = malloc(threads_number * sizeof(thread_args));
    if (args == NULL) {
        perror("Oops, malloc failed");
        exit(1);
    }

    long long chunksize = accuracy / threads_number;
    long long remain = accuracy - chunksize * threads_number;
    int code;
    long long start = 0;
    long long end;

    for (int i = 0; i < threads_number; i++) {
        args[i].start = start;
        end = start + chunksize;
        if (remain > 0) {
            end += 1;
            remain -= 1;
        }
        args[i].end = end;
        start = end;

        code = pthread_create(&threads[i], NULL, thread_body, (void*) (args + i));

        if (code != 0) {
            char buf[256];
            strerror_r(code, buf, sizeof(buf));
            fprintf(stderr, "Unable to create a thread: %s\n", buf);
            for (int j = 0; j < i; j++) {
                pthread_cancel(threads[j]);
            }
            free(threads);
            free(args);
            exit(1);
        }
    }

    double pi = 0.0;
    for (int i = 0; i < threads_number; i++) {
        pthread_join(threads[i], NULL);
        pi += args[i].partial_sum;
    }

    printf("pi with accuracy %lld is: %.15g\n", accuracy, pi);

    free(threads);
    free(args);
    exit(0);
}
