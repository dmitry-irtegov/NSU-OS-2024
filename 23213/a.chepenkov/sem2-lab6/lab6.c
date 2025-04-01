#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_LINES 100
#define COEFF_USLEEP 10000

void* thread_func(void* arg) {
    char* str = (char*)arg;
    size_t len = strlen(str);
    usleep(len * COEFF_USLEEP);
    printf("%s\n", str);
    return NULL;
}

int main(void) {
    pthread_t threads[MAX_LINES];
    int count = 0;
    char buffer[BUFSIZ];
    char* string_arr[MAX_LINES];
    int err;

    while (count < MAX_LINES && fgets(buffer, sizeof(buffer), stdin)) {

        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }

        char* line = strdup(buffer);
        if (!line) {
            perror("strdup");
            exit(EXIT_FAILURE);
        }
        string_arr[count++] = line;
    }

    printf("\n");
    for (int i = 0; i < count; i++) {
        err = pthread_create(&threads[i], NULL, thread_func, string_arr[i]);
        if (err != 0) {
            fprintf(stderr, "Create error: %s\n", strerror(err));
            exit(EXIT_FAILURE);
        }
    }


    for (int i = 0; i < count; i++) {
        err = (pthread_join(threads[i], NULL));
        if (err != 0) {
            fprintf(stderr, "Join error: %s\n", strerror(err));
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}
