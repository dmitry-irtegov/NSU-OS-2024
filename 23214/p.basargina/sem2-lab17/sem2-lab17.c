#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#define CHUNK_SIZE 80

typedef struct Node {
    char *str;
    struct Node *next;
} Node;
Node *head = NULL;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void list_add(char *s) {
    pthread_mutex_lock(&mutex);

    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        perror("Malloc error on newNode");
        exit(1);
    }

    newNode->str = strdup(s);
    if (newNode->str == NULL) {
        perror("Strdup error");
        exit(1);
    }

    newNode->next = head;
    head = newNode;

    pthread_mutex_unlock(&mutex);
}

void list_print() {
    pthread_mutex_lock(&mutex);

    Node *curr = head;
    printf("List state:\n");
    while (curr != NULL) {
        printf("%s\n", curr->str);
        curr = curr->next;
    }

    pthread_mutex_unlock(&mutex);
}

void list_free() {
    pthread_mutex_lock(&mutex);

    Node *curr = head;
    while (curr != NULL) {
        Node *next = curr->next;
        free(curr->str);
        free(curr);
        curr = next;
    }

    pthread_mutex_unlock(&mutex);
}

void bubble_sort() {
    pthread_mutex_lock(&mutex);

    if (head == NULL) {
        pthread_mutex_unlock(&mutex);
        return;
    }
    int swapped;
    Node *ptr;
    Node *lptr = NULL;
    do {
        swapped = 0;
        ptr = head;
        while (ptr->next != lptr) {
            if (strcmp(ptr->str, ptr->next->str) > 0) {
                char *temp = ptr->str;
                ptr->str = ptr->next->str;
                ptr->next->str = temp;
                swapped = 1;
            }
            ptr = ptr->next;
        }
        lptr = ptr;
    } while (swapped);

    pthread_mutex_unlock(&mutex);
}

void process_input(char *input) {
    size_t len = strlen(input);
    for (size_t i = 0; i < len; i += CHUNK_SIZE) {
        char chunk[CHUNK_SIZE + 1];
        size_t chunk_len = (len - i >= CHUNK_SIZE) ? CHUNK_SIZE : len - i;
        strncpy(chunk, input + i, chunk_len);
        chunk[chunk_len] = '\0';
        list_add(chunk);
    }
}

void* thread_function(void *arg) {
    while (1) {
        sleep(5);
        bubble_sort();
    }
    return NULL;
}

int main() {
    pthread_t thread;
    int errCode;

    if ((errCode = pthread_create(&thread, NULL, thread_function, NULL)) != 0) {
        char* buf = strerror(errCode);
        fprintf(stderr, "Failed to create thread: %s\n", buf);
        exit(1);
    }

    char* input;
    size_t len;
    ssize_t read;

    while (1) {
        printf("Enter string (empty string to print list):\n");

        if ((read = getline(&input, &len, stdin)) == -1) {
            printf("Finish\n");
            break;
        }

        if (read > 1) {
	    if (input[read-1] == '\n') {
	        input[read-1] = '\0';
	    }
            process_input(input);
        } else if (read == 1) {
            list_print();
        }
    }

    pthread_cancel(thread);
    pthread_join(thread, NULL);
    list_free();
    free(input);
    pthread_mutex_destroy(&mutex);

    exit(0);
}