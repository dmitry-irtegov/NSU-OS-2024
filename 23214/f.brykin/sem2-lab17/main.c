#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_LINE_LENGTH 80

typedef struct Node {
    char *data;
    struct Node *next;
} Node;

Node *head = NULL;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void add_to_list(const char *str) {
    int len = strlen(str);
    int pos = 0;

    while (pos < len) {
        Node *new_node = (Node *)malloc(sizeof(Node));
        if (!new_node) {
            perror("malloc failed");
            exit(EXIT_FAILURE);
        }

        int part_len = len - pos;
        if (part_len > MAX_LINE_LENGTH) {
            part_len = MAX_LINE_LENGTH;
        }

        new_node->data = (char *)malloc(part_len + 1);
        if (!new_node->data) {
            perror("malloc failed");
            free(new_node);
            exit(EXIT_FAILURE);
        }
        strncpy(new_node->data, str + pos, part_len);
        new_node->data[part_len] = '\0';

        pthread_mutex_lock(&mutex);
        new_node->next = head;
        head = new_node;
        pthread_mutex_unlock(&mutex);
        pos += part_len;
    }
}

void print_list() {
    pthread_mutex_lock(&mutex);
    printf("Current list:\n");
    Node *current = head;
    while (current != NULL) {
        printf("%s\n", current->data);
        current = current->next;
    }
    printf("-----------------------------------------------------------------------------\n");
    pthread_mutex_unlock(&mutex);
}

void free_list() {
    pthread_mutex_lock(&mutex);
    Node *current = head;
    while (current != NULL) {
        Node *temp = current;
        current = current->next;
        free(temp->data);
        free(temp);
    }
    head = NULL;
    pthread_mutex_unlock(&mutex);
}

void bubble_sort() {
    pthread_mutex_lock(&mutex);
    if (head == NULL || head->next == NULL) {
        pthread_mutex_unlock(&mutex);
        return;
    }

    int swapped;
    Node *ptr1;
    Node *lptr = NULL;

    do {
        swapped = 0;
        ptr1 = head;

        while (ptr1->next != lptr) {
            if (strcmp(ptr1->data, ptr1->next->data) > 0) {
                char *temp = ptr1->data;
                ptr1->data = ptr1->next->data;
                ptr1->next->data = temp;
                swapped = 1;
            }
            ptr1 = ptr1->next;
        }
        lptr = ptr1;
    } while (swapped);

    pthread_mutex_unlock(&mutex);
}

void *sort_thread(void *arg) {
    while (1) {
        sleep(5);
        bubble_sort();
    }
    return NULL;
}

int main() {
    pthread_t thread;
    char *input = NULL;
    size_t len = 0;
    ssize_t read;

    if (pthread_create(&thread, NULL, sort_thread, NULL) != 0) {
        perror("pthread_create failed");
        return EXIT_FAILURE;
    }

    printf("Enter strings (empty string to print list, Ctrl+D to exit):\n");

    while ((read = getline(&input, &len, stdin)) != -1) {
        input[strcspn(input, "\n")] = '\0';

        if (input[0] == '\0') {
            print_list();
        } else {
            add_to_list(input);
        }
    }

    free(input);
    free_list();

    pthread_cancel(thread);
    pthread_join(thread, NULL);

    pthread_mutex_destroy(&mutex);

    return EXIT_SUCCESS;
}
