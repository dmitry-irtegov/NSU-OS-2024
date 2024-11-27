#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

typedef struct Node {
    char *data;
    struct Node *next;
} Node;

Node* addNode(Node *head, char *str, size_t len) {
    Node *newNode = (Node*)malloc(sizeof(Node));
    if(newNode == NULL) {
        perror("Memory allocation error");
        exit(EXIT_FAILURE);
    }
    newNode->data = (char*)malloc(len);
    if(newNode->data == NULL) {
        perror("Memory allocation error");
        exit(EXIT_FAILURE);
    }
    strncpy(newNode->data, str, len);
    newNode->next = NULL;
    if(head == NULL) {
        return newNode;
    }
    Node *curr = head;
    while (curr->next != NULL) {
        curr = curr->next;
    }
    curr->next = newNode;
    return head;
}

void freeList(Node *node) {
    Node *curr = node;
    while (curr != NULL) {
        Node *next = curr->next;
        free(curr->data);
        free(curr);
        curr = next;
    }
}

int main() {
    char str[BUFSIZ];
    Node *node = NULL;
    int flag = 0;
    while (1) {
        if (fgets(str, sizeof(str), stdin) == NULL) {
            if(feof(stdin)) {
                printf("EOF detected.\n");
                return 1;
            } else if(ferror(stdin)) {
                perror("Reading fail");
                exit(1);
            }
        }
        size_t len = strlen(str) + 1;
        if (str[0] == '.' && !flag) {
            break;
        }
        if (str[len - 2] != '\n') {
            flag = 1;
        } else {
            flag = 0;
        }
        node = addNode(node, str, len);
    }
    printf("\nEntered strings:\n");
    Node *curr = node;
    while (curr != NULL) {
        printf("%s", curr->data);
        curr = curr->next;
    }
    freeList(node);
    return 0;
}
