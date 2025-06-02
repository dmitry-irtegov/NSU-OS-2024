#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_STRINGS 100

typedef struct {
    char* str;
    size_t length;
} ThreadData;

void* sleep_and_print(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    usleep(data->length * 100000);
    printf("%s\n", data->str);
    free(data->str);
    return NULL;
}

int main() {
    ThreadData thread_data[MAX_STRINGS];
    pthread_t threads[MAX_STRINGS];
    int count = 0;
    char* line = NULL;
    size_t len = 0;
    ssize_t read;

    while (count < MAX_STRINGS && (read = getline(&line, &len, stdin)) != -1) {
        if (read > 0 && line[read - 1] == '\n') {
            line[read - 1] = '\0';
            read--;
        }

        char* str_copy = malloc(read + 1);
        if (!str_copy) {
            perror("malloc failed");
            exit(EXIT_FAILURE);
        }
        strcpy(str_copy, line);

        thread_data[count].str = str_copy;
        thread_data[count].length = read;
        count++;
    }
    free(line);
    printf("\n");
    for (int i = 0; i < count; i++) {
        if (pthread_create(&threads[i], NULL, sleep_and_print, &thread_data[i])) {
            perror("pthread_create failed");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < count; i++) {
        pthread_join(threads[i], NULL);
    }
    return 0;
}