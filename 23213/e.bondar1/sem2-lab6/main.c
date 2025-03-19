#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#define BILL 1000000000
#define COEFF 100
#define MAX_LINES 100
#define MAX_LINE_LENGTH 1024

typedef struct {
    char line[MAX_LINE_LENGTH];
    int length;
} ThreadData;

void* sleep_and_print(void* arg) {
    ThreadData* data = (ThreadData*)arg;

    long long total_ns = data->length * 100000LL * COEFF;

    struct timespec time;
    time.tv_sec = total_ns / BILL;
    time.tv_nsec = total_ns % BILL;

    nanosleep(&time, NULL);

    printf("%d \"%s\"\n", data->length, data->line);
    return NULL;
}

int main(void) {
    pthread_t threads[MAX_LINES];
    ThreadData* thread_data[MAX_LINES];

    char buffer[MAX_LINE_LENGTH];
    int line_count = 0;

    while (fgets(buffer, sizeof(buffer), stdin) != NULL && line_count < MAX_LINES) {
        int len = strlen(buffer);

        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
            len--;
        }

        thread_data[line_count] = (ThreadData*)malloc(sizeof(ThreadData));
        if (!thread_data[line_count]) {
            perror("Failed to allocate memory");
            exit(EXIT_FAILURE);
        }

        strncpy(thread_data[line_count]->line, buffer, len);
        thread_data[line_count]->length = len;

        line_count++;
    }

    printf("Starting threads\n");

    for (int i = 0; i < line_count; i++) {
        int ret = pthread_create(&threads[i], NULL, sleep_and_print, thread_data[i]);
        if (ret != 0) {
            fprintf(stderr, "Failed to create thread: %s\n", strerror(ret));
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < line_count; i++) {
        pthread_join(threads[i], NULL);
        free(thread_data[i]);
    }

    exit(EXIT_SUCCESS);
}
