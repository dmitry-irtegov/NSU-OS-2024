#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#define BUFF_SIZE 80
#define SORT_INTERVAL 5

typedef struct Node_t {
    char buff[BUFF_SIZE + 1];
    struct Node_t *next, *prev;
} Node;

Node* list_head = NULL;
int list_size = 0;
pthread_mutex_t list_lock;

void add_to_list(char* string) {
    pthread_mutex_lock(&list_lock);
    
    Node* new_node = (Node*) calloc(1, sizeof(Node));
    
    strncpy(new_node->buff, string, BUFF_SIZE);
    new_node->buff[BUFF_SIZE] = '\0';

    if (list_head != NULL) {
        list_head->prev = new_node;
        new_node->next = list_head;
    }

    list_head = new_node;
    list_size++;

    pthread_mutex_unlock(&list_lock);
}

void swap_with_next(Node* node) {
    pthread_mutex_lock(&list_lock);

    if (node == NULL || node->next == NULL) {
        return;
    }

    Node* next_node = node->next;

    if (list_head == node) {
        list_head = next_node;
    }
    if (next_node->next != NULL) {
        next_node->next->prev = node;
    }
    if (node->prev != NULL) {
        node->prev->next = next_node;
    }

    node->next = next_node->next;
    next_node->next = node;
    next_node->prev = node->prev;
    node->prev = next_node;

    pthread_mutex_unlock(&list_lock);
}

void sort_list() {
    pthread_mutex_lock(&list_lock);
    
    printf("Sorting was started\n");

    for (int i = 0; i < list_size; i++) {
        for (Node* node = list_head; (node != NULL) && (node->next != NULL); 
                node = node->next) {
            if (strcmp(node->buff, node->next->buff) > 0) {
                swap_with_next(node);
            }
        }
    }

    printf("Sorting was finished\n");

    pthread_mutex_unlock(&list_lock);
}

void print_list() {
    pthread_mutex_lock(&list_lock);
    
    printf("Current list state:\n");
    for (Node* node = list_head; node != NULL; node = node->next) {
        printf("%s\n", node->buff);
    }
    printf("End of current list state\n");

    pthread_mutex_unlock(&list_lock);
}

void *sorter_routine(void* arg) {
    while (1) {
        sleep(SORT_INTERVAL);
         
        sort_list();
    }
}

int main() {
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&list_lock, &attr);
    pthread_mutexattr_destroy(&attr);

    pthread_t thread;
    pthread_create(&thread, NULL, sorter_routine, NULL);

    char *input_line = NULL, buff[BUFF_SIZE + 1];
    size_t input_line_size = 0;

    ssize_t bytes_read;


    while((bytes_read = getline(&input_line, &input_line_size, stdin)) > 0) {
        if (input_line[0] == '\n') {
            print_list();
        } else {
            input_line[bytes_read - 1] = '\0';
            for (ssize_t start_index = 0; start_index < bytes_read;
                    start_index += BUFF_SIZE) {
                strncpy(buff, &input_line[start_index], BUFF_SIZE);
                add_to_list(buff);
            }
        }

        free(input_line);
        input_line = NULL;
    }
    

    pthread_mutex_destroy(&list_lock);
    exit(EXIT_SUCCESS);
}
