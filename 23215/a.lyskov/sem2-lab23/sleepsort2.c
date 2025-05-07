#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#define SLEEP_KOEFF 100000
#define MAX_LINES 100
#define MAX_LINE_LENGTH 1024

typedef struct Node {
    char *str;
    struct Node *next;
} Node;

typedef struct List {
    Node *head;
    Node *tail;
} List;

List sorted_list = {NULL, NULL};
pthread_mutex_t list_mutex = PTHREAD_MUTEX_INITIALIZER;

void add_to_list(char *str) {
    Node *new_node = malloc(sizeof(Node));
    new_node->str = strdup(str);
    new_node->next = NULL;
    
    pthread_mutex_lock(&list_mutex);
    
    if (sorted_list.head == NULL) {
        sorted_list.head = sorted_list.tail = new_node;
    } else {
        sorted_list.tail->next = new_node;
        sorted_list.tail = new_node;
    }
    
    pthread_mutex_unlock(&list_mutex);
}

void *sort_thread(void *arg) {
    char *str = (char *)arg;
    size_t len = strlen(str);
    
    usleep(len * SLEEP_KOEFF);
    add_to_list(str);
    
    return NULL;
}

void print_list() {
    Node *current = sorted_list.head;
    while (current != NULL) {
        printf("%s\n", current->str);
        Node *temp = current;
        current = current->next;
        free(temp->str);
        free(temp);
    }
    sorted_list.head = sorted_list.tail = NULL;
}

int main() {
    char *lines[MAX_LINES];
    int count = 0;
    pthread_t threads[MAX_LINES];
    
    char buffer[MAX_LINE_LENGTH];
    while (fgets(buffer, sizeof(buffer), stdin) && count < MAX_LINES) {
        buffer[strcspn(buffer, "\n")] = '\0';
        if (buffer[0] == '\0') break;
        lines[count] = strdup(buffer);
        count++;
    }
    
    for (int i = 0; i < count; i++) {
        pthread_create(&threads[i], NULL, sort_thread, lines[i]);
    }
    for (int i = 0; i < count; i++) {
        pthread_join(threads[i], NULL);
    }
    print_list();
    
    for (int i = 0; i < count; i++) {
        free(lines[i]);
    }
    return 0;
}
