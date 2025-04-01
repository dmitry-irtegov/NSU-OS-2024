#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

typedef struct Args {
    int start;
    int end;
    double res;
} Args;

void err_handler(char *msg, int errID) {
    fprintf(stderr, "%s %s\n", msg, strerror(errID));
}

void *pi_serial(void *data) {
    Args *args = (Args *) data;
    double result = 0;
    for (int i = args->start; i < args->end; i++) {
        result += 1.0 / (i * 4.0 + 1.0);
        result -= 1.0 / (i * 4.0 + 3.0);
    }
    args->res = result;
    pthread_exit(NULL);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "ERROR: not enough arguments. Try %s <num_of_threads>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int errID = 0;
    int num_threads = atoi(argv[1]);
    int num_elem_row = 200000000;
    int steps = num_elem_row / num_threads;
    int rest = num_elem_row % num_threads;
    double result = 0;
    pthread_attr_t attr;

    Args *args = malloc(sizeof(Args) * num_threads);
    if (args == NULL) {
        perror("ERROR: failed to allocate memory!");
        exit(EXIT_FAILURE);
    }

    pthread_t *threads = malloc(sizeof(pthread_t) * num_threads);
    if (threads == NULL) {
        perror("ERROR: failed to allocate memory!");
        free(args);
        exit(EXIT_FAILURE);
    }

    if ((errID = pthread_attr_init(&attr) != 0)) {
        err_handler("ERROR: failed to init attr.", errID);
        free(threads);
        free(args);
        exit(EXIT_FAILURE);
    }

    args[0].start = 0;
    args[0].end = steps + rest;
    for (int i = 1; i < num_threads; i++) {
        args[i].start = args[i - 1].end;
        args[i].end = args[i - 1].end + steps + rest;
    }

    if (args[num_threads - 1].end > num_elem_row) {
        args[num_threads - 1].end = num_elem_row;
    }

    for (int i = 0; i < num_threads; i++) {
        if ((errID = pthread_create(&(threads[i]), &attr, &pi_serial, (void *) &args[i])) != 0) {
            err_handler("ERROR: failed to create thread.", errID);
            free(threads);
            free(args);
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < num_threads; i++) {
        if ((errID = pthread_join(threads[i], NULL)) != 0) {
            err_handler("ERROR: failed to join thread.", errID);
            free(args);
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < num_threads; i++) {
        result += args[i].res;
    }

    if ((errID = pthread_attr_destroy(&attr)) != 0) {
        err_handler("ERROR: failed to destroy attr.", errID);
        free(threads);
        free(args);
        exit(EXIT_FAILURE);
    }

    result *= 4.0;
    printf("Result: %.5le\n", result);
    free(threads);
    free(args);
    exit(EXIT_SUCCESS);
}