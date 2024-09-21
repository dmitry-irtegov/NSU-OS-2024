#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define BUFF_SIZE 5

typedef struct Node {
    char *data;
    struct Node *next;
} Node;

Node* addNode(Node *head, char *str, size_t len) {
    Node *new = (Node*)malloc(sizeof(Node));
    if(new == NULL) {
        perror("Memory allocation error");
        exit(1);
    }

    new->data = (char*)malloc(len);
    if(new->data == NULL) {
        perror("Memory allocation error");
        exit(1);
    }

    strncpy(new->data, str, len);
    new->next = NULL;

    if(head == NULL) {
        return new;
    }

    Node *curr = head;
    while (curr->next != NULL) {
        curr = curr->next;
    }
    curr->next = new;

    return head;
}

void printList(Node *head) {
    Node *curr = head;
    while (curr != NULL) {
        printf("%s", curr->data);
        curr = curr->next;
    }
}

void freeMem(Node *node) {
    Node *curr = node;
    while (curr != NULL) {
        Node *next = curr->next;
        free(curr->data);
        free(curr);
        curr = next;
    }
}

int main() {
    char str[BUFF_SIZE];
    char *data;
    Node *node = NULL;
    int flag = 0;

    while (1) {
        data = fgets(str, sizeof(str), stdin);
        if (data == NULL) {
            if(feof(stdin)) {
                printf("EOF detected.\n");
                return 1;
            } else if(ferror(stdin)) {
                perror("Reading fail");
                exit(1);
            }
        }

        size_t len = strlen(data) + 1;
        
        if (data[0] == '.' && !flag) {
            break;
        }

        if (data[len - 2] != '\n') {
            flag = 1;
        } else {
            flag = 0;
        }
      
        node = addNode(node, data, len);
    }

    printList(node);
    freeMem(node);
    
    return 0;
}

