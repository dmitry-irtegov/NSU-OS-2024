#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>

typedef struct Node {
    char* value;
    struct Node* next;
} Node;

Node* createNode(char* string) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        perror("Node malloc error");
        exit(-1);
    }
    size_t len = strlen(string) + 1;
    newNode->value = (char*)malloc(len);
    if (newNode->value == NULL) {
        perror("Value malloc error");
        free(newNode);
        exit(-1);
    }
    strncpy(newNode->value, string, len);
    newNode->next = NULL;
    return newNode;
}

Node* addNode(Node* nodeList, char* string) {
    Node* newNode = createNode(string);
    if (nodeList == NULL) {
        return newNode;
    }
    Node* curNode = nodeList;
    while (curNode->next != NULL) {
        curNode = curNode->next;
    }
    curNode->next = newNode;
    return nodeList;
}

void freeNodes(Node* nodeList) {
    Node* curNode = nodeList;
    while (curNode != NULL) {
        Node* nextNode = curNode->next;
        free(curNode->value);
        free(curNode);
        curNode = nextNode;
    }
}

void printNodes(Node* nodeList) {
    Node* curNode = nodeList;
    while (curNode != NULL) {
        printf("%s", curNode->value);
        curNode = curNode->next;
    }
}

int main() {
    Node* nodeList = NULL;
    char msg[5];
    int flag = 0;
    while(1) {
        if (fgets(msg, sizeof(msg), stdin) == NULL) {
            if (ferror(stdin)) {
                exit(-1);
            }
            if (feof(stdin)) {
                break;
            }
        }
        if (msg[0] == '.' && !flag) {
            break;
        }
        if (msg[strlen(msg) - 1] != '\n') {
            flag = 1;
        }
        else {
            flag = 0;
        }
        nodeList = addNode(nodeList, msg);
    }
    printNodes(nodeList);
    freeNodes(nodeList);
    exit(0);
}