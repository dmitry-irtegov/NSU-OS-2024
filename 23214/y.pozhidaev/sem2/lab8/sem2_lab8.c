#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define num_steps 200000000

typedef struct ThreadStruct {
    int start;
    int end;
    double result;
} ThreadStruct;

void *thread_body(void *param) {
    ThreadStruct *thread_struct = (ThreadStruct *) param;
    double pi = 0;

    for (int i = thread_struct->start; i < thread_struct->end; i++) {
        pi += 1.0 / (i * 4.0 + 1.0);
        pi -= 1.0 / (i * 4.0 + 3.0);
    }

    thread_struct->result = pi;
    pthread_exit(thread_struct);
}

int main(int argc, char **argv) {
    int code;
    char buf[256];
    double pi = 0;
    int numberThreads = 1;

    if (argc > 1) {
        numberThreads = atoi(argv[1]);
        if (numberThreads <= 0) {
            fprintf(stderr, "Number of threads must be positive\n");
            exit(1);
        }
    } else {
        fprintf(stderr, "Print number of threads\n");
        exit(1);
    }

    pthread_t *threads = malloc(sizeof(pthread_t) * numberThreads);
    if (threads == NULL) {
        perror("Malloc error");
        exit(1);
    }

    ThreadStruct *structs = malloc(sizeof(ThreadStruct) * numberThreads);
    if (structs == NULL) {
        perror("Malloc error");
        free(threads);
        exit(1);
    }

    int number_of_numbers = num_steps / numberThreads;
    int remainder = num_steps % numberThreads;
    int start = 0;

    for (int i = 0; i < numberThreads; i++) {
        structs[i].start = start;
        structs[i].end = start + number_of_numbers;
        structs[i].result = 0.0;

        if (remainder > 0) {
            remainder--;
            structs[i].end++;
        }
        start = structs[i].end;

        code = pthread_create(&threads[i], NULL, thread_body, (void *) &structs[i]);
        if (code != 0) {
            strerror_r(code, buf, sizeof(buf));
            fprintf(stderr, "%s: creating thread %d: %s\n", argv[0], i, buf);
            free(threads);
            free(structs);
            exit(1);
        }
    }

    for (int i = 0; i < numberThreads; i++) {
        code = pthread_join(threads[i], NULL);
        if (code != 0) {
            strerror_r(code, buf, sizeof(buf));
            fprintf(stderr, "%s: joining thread %d: %s\n", argv[0], i, buf);
            free(threads);
            free(structs);
            exit(1);
        }
        pi += structs[i].result;
    }

    pi = pi * 4.0;
    printf("pi done - %.15g \n", pi);

    free(threads);
    free(structs);
    return 0;
}
