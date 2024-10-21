#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef LEN_BUFFER
#define LEN_BUFFER 10
#endif

typedef struct Node{
    char* string;
    struct Node* next;
}Node;

Node* addNode(Node* prev, char* string){
    Node* ans;
    ans = malloc(sizeof(Node));
    if (ans == NULL){
        return NULL;
    }
    if (prev != NULL){
        prev->next = ans;
    }
    
    ans->next = NULL;
    ans->string = string;
    return ans;
}

void printList(Node* cur){
    if (cur != NULL){
        printf(cur->string);
        printList(cur->next);
    }
}

void freeList(Node* cur) {
    if (cur != NULL){
        freeList(cur->next);
        free(cur);
    }
} 

void exitProgram(int code, char* message){
    if (message != NULL){
        perror(message);
    }
    exit(code);
}

int getListFromUser(Node** startList) {
    char buffer[LEN_BUFFER], flag_fullstop = 0, flag_start = 1;
    char* string = NULL;
    Node *prev = NULL, *start = NULL;

    while (fgets(buffer, LEN_BUFFER, stdin)){
        if (buffer[0] == '.') {
            if (strlen(buffer) <= 2) {
                break;
            }
            string = strdup(&buffer[1]);
            flag_fullstop = 1;
        }
        else {
            string = strdup(buffer);
        }

        if (string == NULL) {
            return 1;
        }

        while (buffer[strlen(buffer)-1] != '\n'){
            fgets(buffer, LEN_BUFFER, stdin);
            if (ferror(stdin)) {
                return 1;
            }

            string = realloc(string, strlen(buffer) + strlen(string) + 1);
            if (string == NULL) {
                return 1;
            }

            strcat(string, buffer);
        }
        
        prev = addNode(prev, string);

        if (prev == NULL) {
            return 1;
        }

        if (flag_start) {
            flag_start = 0;
            start = prev;
        }

        if (flag_fullstop){
            break;
        }

    }

    if (ferror(stdin)){
        return 1;
    }

    if (feof(stdin)){
        return 2;
    }


    *startList = start;
    return 0;
}

int main(){
    Node* start = NULL;
    Node** startPointer = &start;

    switch (getListFromUser(startPointer)) {
        case 1: 
            exitProgram(EXIT_FAILURE, "getListFromUser failed");
            break;
        case 2:
            fprintf(stderr, "getListFromUser failed: EOF\n");
            exitProgram(EXIT_FAILURE, NULL);
    }

    puts("strings:");
    printList(start);

    freeList(start);

    exitProgram(EXIT_SUCCESS, NULL);
}