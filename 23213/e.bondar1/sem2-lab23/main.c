#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define MILL 1000000LL
#define MAX_LINES 100
#define MAX_LINE_LENGTH 1024
#define SLEEP_COEFF 10000

typedef struct ListNode {
    char line[MAX_LINE_LENGTH];
    int length;
    struct ListNode *next;
} ListNode;

typedef struct {
    ListNode *head;
    ListNode *tail;
    pthread_mutex_t list_mutex;
} SharedData;

typedef struct {
    char line[MAX_LINE_LENGTH];
    int length;
    SharedData *shared;
} ThreadData;

void* sleep_and_add_to_list(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    SharedData* shared = data->shared;

    long long sleep_us = (long long)data->length * SLEEP_COEFF;

    struct timespec sleep_time;
    sleep_time.tv_sec = sleep_us / MILL;
    sleep_time.tv_nsec = (sleep_us % MILL) * 1000LL;
    nanosleep(&sleep_time, NULL);

    pthread_mutex_lock(&shared->list_mutex);

    ListNode *new_node = malloc(sizeof(ListNode));
    if (!new_node) {
        perror("malloc failed for list node");
        exit(EXIT_FAILURE);
    }

    strncpy(new_node->line, data->line, MAX_LINE_LENGTH);
    new_node->length = data->length;
    new_node->next = NULL;

    if (shared->tail == NULL) {
        shared->head = new_node;
        shared->tail = new_node;
    } else {
        shared->tail->next = new_node;
        shared->tail = new_node;
    }

    pthread_mutex_unlock(&shared->list_mutex);
    return NULL;
}

int main(void) {
    pthread_t threads[MAX_LINES];
    ThreadData* thread_data[MAX_LINES];
    SharedData shared_data;

    char buffer[MAX_LINE_LENGTH];
    int line_count = 0;
    int ret;

    shared_data.head = NULL;
    shared_data.tail = NULL;

    ret = pthread_mutex_init(&shared_data.list_mutex, NULL);
    if (ret != 0) {
        fprintf(stderr, "Mutex init failed: %s\n", strerror(ret));
        exit(EXIT_FAILURE);
    }

    while (fgets(buffer, sizeof(buffer), stdin) != NULL && line_count < MAX_LINES) {
        size_t len = strlen(buffer);

        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
            len--;
        }

        thread_data[line_count] = (ThreadData*)malloc(sizeof(ThreadData));
        if (!thread_data[line_count]) {
            perror("malloc failed for thread data");
            pthread_mutex_destroy(&shared_data.list_mutex);
            exit(EXIT_FAILURE);
        }

        strncpy(thread_data[line_count]->line, buffer, len);
        thread_data[line_count]->length = len;
        thread_data[line_count]->shared = &shared_data;

        line_count++;
    }

    printf("Starting threads\n");

    for (int i = 0; i < line_count; i++) {
        ret = pthread_create(&threads[i], NULL, sleep_and_add_to_list, thread_data[i]);
        if (ret != 0) {
            fprintf(stderr, "Failed to create thread: %s\n", strerror(ret));
            pthread_mutex_destroy(&shared_data.list_mutex);
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < line_count; i++) {
        pthread_join(threads[i], NULL);
        free(thread_data[i]);
    }

    printf("--- Sorted Lines ---\n");
    ListNode *current = shared_data.head;
    ListNode *next_node;
    while (current != NULL) {
        printf("%d \"%s\"\n", current->length, current->line);
        next_node = current->next;
        free(current);
        current = next_node;
    }
    printf("--- End of List ---\n");

    pthread_mutex_destroy(&shared_data.list_mutex);
    return EXIT_SUCCESS;
}
