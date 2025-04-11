#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>
#include <signal.h>

#define MAX_CHUNK_SIZE 80

typedef struct Node {
    char *data;
    struct Node *next;
} Node;

Node *head = NULL;
pthread_mutex_t mutex;
volatile sig_atomic_t stop_flag = 0;

void handle_signal(int signum) {
    if (signum == SIGINT) {
        stop_flag = 1;
    }
}

void add_to_list(const char *str) {
    Node *new_node = (Node *)malloc(sizeof(Node));
    if (!new_node) {
        perror("ERROR: Allocation failed");
        exit(EXIT_FAILURE);
    }
    new_node->data = strdup(str);
    new_node->next = NULL;
    assert(pthread_mutex_lock(&mutex) == 0);
    new_node->next = head;
    head = new_node;
    assert(pthread_mutex_unlock(&mutex) == 0);
}

void split_and_add(const char *input) {
    size_t len = strlen(input);
    for (size_t i = 0; i < len; i += MAX_CHUNK_SIZE) {
        char temp[MAX_CHUNK_SIZE + 1];
        strncpy(temp, input + i, MAX_CHUNK_SIZE);
        temp[MAX_CHUNK_SIZE] = '\0';
        add_to_list(temp);
    }
}

void print_list() {
    assert(pthread_mutex_lock(&mutex) == 0);
    Node *current = head;
    printf("Current list state:\n");
    while (current) {
        printf("%s\n", current->data);
        current = current->next;
    }
    printf("-----\n");
    assert(pthread_mutex_unlock(&mutex) == 0);
}

void bubble_sort() {
    if (!head || !head->next) return;

    Node *sorted = NULL;
    Node *current = head;
    while (current) {
        Node *next = current->next;
        if (!sorted || strcmp(current->data, sorted->data) < 0) {
            current->next = sorted;
            sorted = current;
        } else {
            Node *temp = sorted;
            while (temp->next && strcmp(temp->next->data, current->data) < 0) {
                temp = temp->next;
            }
            current->next = temp->next;
            temp->next = current;
        }
        current = next;
    }
    head = sorted;
}

void free_list() {
    assert(pthread_mutex_lock(&mutex) == 0);
    Node *current = head;
    while (current) {
        Node *next = current->next;
        free(current->data);
        free(current);
        current = next;
    }
    head = NULL;
    assert(pthread_mutex_unlock(&mutex) == 0);
}

void *sort_thread() {
    while (!stop_flag) {
        sleep(5);
        assert(pthread_mutex_lock(&mutex) == 0);
        bubble_sort();
        assert(pthread_mutex_unlock(&mutex) == 0);
    }
    return NULL;
}

int main() {
    signal(SIGINT, handle_signal);

    pthread_t thread;
    int errCode;
    pthread_mutexattr_t attr;
    if ((errCode = pthread_mutexattr_init(&attr)) != 0) {
        fprintf(stderr, "ERROR: Mutex attribute initialization failed: %s\n", strerror(errCode));
        exit(-1);
    }
    if ((errCode = pthread_mutex_init(&mutex, &attr)) != 0) {
        fprintf(stderr, "ERROR: Mutex initialization failed: %s\n", strerror(errCode));
        exit(-1);
    }
    if ((errCode = pthread_create(&thread, NULL, sort_thread, NULL)) != 0) {
        fprintf(stderr, "ERROR: Thread creation failed: %s\n", strerror(errCode));
        exit(-1);
    }

    char input[256];
    while (!stop_flag) {
        printf("Enter text: ");
        if (!fgets(input, sizeof(input), stdin)) {
            perror("Input error");
            break;
        }
        size_t len = strlen(input);
        if (len > 0 && input[len - 1] == '\n') {
            input[len - 1] = '\0';
        }
        if (strlen(input) == 0) {
            print_list();
        } else {
            split_and_add(input);
        }
    }
    printf("\nExiting program...\n");
    if ((errCode = pthread_cancel(thread)) != 0) {
        fprintf(stderr, "ERROR: Thread canceling failed: %s\n", strerror(errCode));
    }
    if ((errCode = pthread_join(thread, NULL)) != 0) {
        fprintf(stderr, "ERROR: Thread join failed: %s\n", strerror(errCode));
    }
    free_list();
    if ((errCode = pthread_mutex_destroy(&mutex)) != 0) {
        fprintf(stderr, "ERROR: Mutex destruction failed: %s\n", strerror(errCode));
    }
    pthread_mutexattr_destroy(&attr);
    printf("Memory freed. Exiting safely.\n");
    return 0;
}
