#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFFERSIZE 80

typedef struct Node {
    char *data;
    struct Node *next;
} Node;

Node *head = NULL;
int list_size = 0;
char list_alive = 1;
pthread_mutex_t list_mutex;

void add_line_to_list(char *line) {
    pthread_mutex_lock(&list_mutex);

    char *new_line = (char *)malloc(strlen(line) + 1);
    strcpy(new_line, line);

    Node *new_node = (Node *)malloc(sizeof(Node));

    new_node->data = new_line;
    new_node->next = head;
    head = new_node;

    list_size++;

    pthread_mutex_unlock(&list_mutex);
}

void print_list() {
    pthread_mutex_lock(&list_mutex);

    fputs("------- list data -------\n", stdout);

    for (Node *curr_node = head; curr_node != NULL; curr_node = curr_node->next) {
        fputs(curr_node->data, stdout);
    }

    fputs("------- list data -------\n", stdout);

    pthread_mutex_unlock(&list_mutex);
}

void swap(char **line1, char **line2) {
    char *buff = *line1;
    *line1 = *line2;
    *line2 = buff;
}

void sort_list() {

    if (list_size < 2)
        return;

    Node *curr, *next;

    for (int i = 0; i < list_size - 1; i++) {
        curr = head;
        next = curr->next;
        for (int j = 0; j < list_size - i - 1; j++) {
            if (strcmp(curr->data, next->data) > 0)
                swap(&curr->data, &next->data);
            curr = next;
            next = curr->next;
        }
    }
}

void destroy_list() {

    pthread_mutex_lock(&list_mutex);

    Node *next_node;

    while (head != NULL) {
        next_node = head->next;
        free(head->data);
        free(head);
        head = next_node;
    }

    list_alive = 0;
    
    pthread_mutex_unlock(&list_mutex);
}

void *start_routine(void *param) {
    while (1) {
        sleep(5);
        pthread_mutex_lock(&list_mutex);

        if (list_alive) {
            sort_list();
            pthread_mutex_unlock(&list_mutex);
        } else {
            pthread_mutex_unlock(&list_mutex);
            break;
        }
    }
    
    pthread_exit(NULL);
}

int main() {

    char line[BUFFERSIZE];
    pthread_t thread;

    if (pthread_mutex_init(&list_mutex, NULL) != 0) {
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&thread, NULL, start_routine, NULL) != 0) {
        pthread_mutex_destroy(&list_mutex);
        exit(EXIT_FAILURE);
    }

    while (fgets(line, BUFFERSIZE, stdin) != NULL) {
        if (strlen(line) == 1) {
            print_list();
        } else {
            add_line_to_list(line);
        }
    }

    destroy_list();

    pthread_join(thread, NULL);
    pthread_mutex_destroy(&list_mutex);
    exit(EXIT_SUCCESS);
}
