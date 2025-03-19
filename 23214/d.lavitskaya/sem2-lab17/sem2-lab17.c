#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

#define MAX_LINE_LENGTH 80

typedef struct Node {
    char *data;
    struct Node *next;
} Node;

Node *head = NULL;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 
int stop_sorting = 0; 

void add_to_list(const char *str) {
    Node *new_node = (Node *)malloc(sizeof(Node));
    new_node->data = strdup(str);
    pthread_mutex_lock(&mutex);
    new_node->next = head;
    head = new_node;
    pthread_mutex_unlock(&mutex);
}

void print_list() {
    pthread_mutex_lock(&mutex);
    Node *temp = head;
    printf("Current list:\n");
    while (temp) {
        printf("%s\n", temp->data);
        temp = temp->next;
    }
    pthread_mutex_unlock(&mutex);
}

void bubble_sort_list() {
    if (!head || !head->next) return;
    int swapped;
    do {
        swapped = 0;
        pthread_mutex_lock(&mutex);
        Node *current = head;
        while (current->next) {
            if (strcmp(current->data, current->next->data) > 0) {
                char *temp = current->data;
                current->data = current->next->data;
                current->next->data = temp;
                swapped = 1;
            }
            current = current->next;
        }
        pthread_mutex_unlock(&mutex);
    } while (swapped);
}

void free_list() {
    pthread_mutex_lock(&mutex);
    Node *current = head;
    while (current) {
        Node *temp = current;
        current = current->next;
        free(temp->data);
        free(temp);
    }
    head = NULL;
    pthread_mutex_unlock(&mutex);
}

void handle_signal(int sig) {
    printf("\nReceived signal %d, cleaning up...\n", sig);
    stop_sorting = 1;
    free_list();
    pthread_mutex_destroy(&mutex);
    pthread_exit(NULL);
}

void *sort_thread(void *arg) {
    while (!stop_sorting) {
        sleep(5);
        bubble_sort_list();
    }
    pthread_exit(NULL);
}

int main() {
    signal(SIGINT, handle_signal);

    pthread_t sorter;
    int code;
    code = pthread_create(&sorter, NULL, sort_thread, NULL);
    if (code != 0) {
        char buf[256];
        strerror_r(code, buf, sizeof(buf));
        fprintf(stderr, "Failed creating thread: %s\n",  buf);
        exit(1);
    }
    
    char input[MAX_LINE_LENGTH + 1];
    while (1) {
        printf("Enter a string: ");
        int index = 0;
        char ch;
        while (index < MAX_LINE_LENGTH) {
            ch = getchar();
            if (ch == '\n' || ch == EOF) {
                if (index == 0) {
                    print_list();
                }
                else {
                    input[index] = '\0';
                    add_to_list(input);
                }
                break;
            }

            input[index++] = ch;

            if (index == MAX_LINE_LENGTH) {
                input[index] = '\0';
                add_to_list(input);
                index = 0;
            }
        }
    }
    
}
