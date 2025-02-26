#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_LINES 100
#define COEFF_USLEEP 10000

typedef struct Node {
    char* str;
    struct Node* next;
} Node;

Node* head = NULL;
Node* tail = NULL;
pthread_mutex_t list_mutex = PTHREAD_MUTEX_INITIALIZER;


void* thread_func(void* arg) {
    char* line = (char*)arg;
    size_t len = strlen(line);

    usleep(len * COEFF_USLEEP);

    pthread_mutex_lock(&list_mutex);
    Node* new_node = malloc(sizeof(Node));
    if (!new_node) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    new_node->str = line;
    new_node->next = NULL;


    if (tail == NULL) {
        head = tail = new_node;
    }
    else {
        tail->next = new_node;
        tail = new_node;
    }
    pthread_mutex_unlock(&list_mutex);

    return NULL;
}

void print_list() {
    printf("\n");
    Node* curr = head;
    while (curr) {
        printf("%s\n", curr->str);
        curr = curr->next;
    }
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
            pthread_mutex_destroy(&list_mutex);
            exit(EXIT_FAILURE);
        }
        string_arr[count++] = line;
    }
    for (int i = 0; i < count; i++) {
        err = pthread_create(&threads[i], NULL, thread_func, string_arr[i]);
        if (err != 0) {
            fprintf(stderr, "Create error: %s\n", strerror(err));
            pthread_mutex_destroy(&list_mutex);
            exit(EXIT_FAILURE);
        }
    }


    for (int i = 0; i < count; i++) {
        err = (pthread_join(threads[i], NULL));
        if (err != 0) {
            fprintf(stderr, "Join error: %s\n", strerror(err));
            pthread_mutex_destroy(&list_mutex);
            exit(EXIT_FAILURE);
        }
    }

    print_list();

    Node* curr = head;
    while (curr) {
        Node* temp = curr;
        curr = curr->next;
        free(temp->str);
        free(temp);
    }

    pthread_mutex_destroy(&list_mutex);
    return 0;
}
