#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void *thread_function(void *arg) {
    usleep(strlen((char*)arg) * 10000);
    printf("%s", (char*)arg);
    return NULL;
}

int main(void) {
    int n, code;
    if (scanf("%d\n", &n) != 1) {
        fprintf(stderr, "Failed to read number of lines.\n");
        return EXIT_FAILURE;
    }
    if (n <= 0) {
        fprintf(stderr, "Invalid number of lines.\n");
        return EXIT_FAILURE;
    }
    char **lines = calloc(n, sizeof(char *));
    if (lines == NULL) {
        perror("calloc failed for lines");
        return EXIT_FAILURE;
    }
    for (int i = 0; i < n; i++) {
        size_t bufsize = 0;
        if (getline(&lines[i], &bufsize, stdin) == -1) {
            fprintf(stderr, "Failed to read line %d.\n", i + 1);
            for (int j = 0; j <= i; j++) {
                free(lines[j]);
            }
            free(lines);
            return EXIT_FAILURE;
        }
    }
    pthread_t *threads = malloc(n * sizeof(pthread_t));
    if (threads == NULL) {
        perror("malloc failed for threads");
        for (int i = 0; i < n; i++) {
            free(lines[i]);
        }
        free(lines);
        return EXIT_FAILURE;
    }
    for (int i = 0; i < n; i++) {
        if ((code = pthread_create(&threads[i], NULL, thread_function, lines[i])) != 0) {
            fprintf(stderr, "Failed to create thread: %d.\n", code);
            for (int j = 0; j < i; j++) {
                pthread_join(threads[j], NULL);
            }
            for (int j = 0; j < n; j++) {
                free(lines[j]);
            }
            free(lines);
            free(threads);
            return EXIT_FAILURE;
        }
    }
    int is_success = 1;
    for (int i = 0; i < n; i++) {
        if ((code = pthread_join(threads[i], NULL)) != 0) {
            fprintf(stderr, "pthread_join failed: %d.\n", code);
            is_success = 0;
        }
    }
    for (int i = 0; i < n; i++) {
        free(lines[i]);
    }
    free(lines);
    free(threads);
    return is_success ? EXIT_SUCCESS : EXIT_FAILURE;
}
