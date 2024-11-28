#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node{
    char* str;
    struct Node* next;
} Node;

void printWithFree(Node *node) {
    Node *curr = node;
    Node* next = NULL;
    while (curr != NULL) {
        next = curr->next;
        fprintf(stdout, "%s", curr->str);
        free(curr->str);
        free(curr);
        curr = next;
    }
}

int main() {
    char buffer[64];
    Node* curr = NULL;
    Node* prev = NULL;
    Node* head = NULL;
    int flag = 0;

    while(1) {
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            if(feof(stdin)) {
                fprintf(stderr, "EOF sended\n");
                return 1;
            }
            
            else {
                perror("Reading finished with error");
                return 1;
            }
        }

        int len = strlen(buffer) + 1;
        
        if (buffer[0] == '.' && !flag) {
            break;
        }

        if (buffer[len - 2] != '\n') {
            flag = 1;
        }
        else {
            flag = 0;
        }
      
        curr = malloc(sizeof(Node));
        if (curr == NULL) {
            perror("malloc finished with error");
            return 1;
        }

        curr->str = malloc(len * sizeof(char)); 
        if (curr->str == NULL) {
            perror("malloc finished with error");
            return 1;
        }

        curr->next = NULL;
        strncpy(curr->str, buffer, len);

        if (prev == NULL){
            head = curr;
            prev = curr;
        }
        else {
            prev->next = curr;
            prev = curr;
        }
    }

    printWithFree(head);
    return 0;
}
