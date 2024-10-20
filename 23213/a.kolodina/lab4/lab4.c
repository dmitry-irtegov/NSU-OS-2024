#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define BUFFERSIZE 1024

typedef struct Node{
    char* data;
    struct Node* next;
} Node;

Node* add_node(Node* head, char* data, size_t size) {
    Node* new_node = (Node*) malloc(sizeof(Node));
    if (new_node == NULL) {
        perror("malloc error");
        exit(1);
    }
    new_node->data = (char*) malloc(size*sizeof(char)); 
    if (new_node->data == NULL) {
        perror("malloc error");
        exit(1);
    }
    new_node->next = NULL;
    strncpy(new_node->data, data, size);   
    if (head == NULL) {
        return new_node;
    }
    Node* curr = head;
    while (curr->next != NULL) {
        curr = curr->next;
    }
    curr->next = new_node;
    return head;
}

void free_memory(Node* head) {
    Node* curr = head;
    Node* prev = curr;
    while (curr!= NULL) {
        free(curr->data);
        prev = curr;
        curr = curr->next;
        free(prev);
    }
}

int main(int argc, char* argv[]) {
    Node* head = NULL;
    int newline = 0;
    char* buffer = (char*) malloc(sizeof(char) * BUFFERSIZE);
    if (buffer == NULL) {
        perror("malloc error");
        return 1;
    }
    size_t size = 0;
    while(1) {
        if (fgets(buffer, BUFFERSIZE, stdin) == NULL) {
            if (feof(stdin)) {
                break;
            }
            if (ferror(stdin)) {
                perror("fgets error");
                return 1;
            }
        }
        if (buffer[0] == '.' && newline) {
            break;
        }
        size = strlen(buffer) + 1;
        head = add_node(head, buffer, size);
        if (buffer[size - 2] == '\n') {
            newline = 1;
        } else {
            newline = 0;
        }
    }
    Node* curr = head;
    while (curr!= NULL) {
        printf("%s", curr->data);
        curr = curr->next;
    }
    free(buffer);
    free_memory(head);
    return 0;
}
