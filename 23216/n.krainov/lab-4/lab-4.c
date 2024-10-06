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

void printListAndFree(Node* cur){
    if (cur != NULL){
        printf(cur->string);
        printListAndFree(cur->next);
        free(cur);
    }
}

void exitProgram(int code, char* message){
    if (message != NULL){
        perror(message);
    }
    exit(code);
}

int main(int argc, char* argv[]){
    char buffer[LEN_BUFFER], flag_fullstop = 0;
    char* string = NULL;
    Node *start = NULL, *prev = NULL;
    while (fgets(buffer, LEN_BUFFER, stdin)){
        if (buffer[0] == '.'){
            if (strlen(buffer) == 2) {
                break;
            }
            string = strdup(&buffer[1]);
            flag_fullstop = 1;
        }
        else{
            string = strdup(buffer);
        }
        if (string == NULL){
            exitProgram(EXIT_FAILURE, "strdup failed");
        }
        while (buffer[strlen(buffer)-1] != '\n'){
            fgets(buffer, LEN_BUFFER, stdin);
            if (ferror(stdin)){
                exitProgram(EXIT_FAILURE, "fgets failed");
            }
            string = realloc(string, strlen(buffer) + strlen(string) + 1);
            if (string == NULL){
                exitProgram(EXIT_FAILURE, "realloc failed");
            }
            strcat(string, buffer);
        }
        
        prev = addNode(prev, string);

        if (prev == NULL){
            exitProgram(EXIT_FAILURE, "addNode failed");
        }

        if (start == NULL) {
            start = prev;
        }

        if (flag_fullstop){
            break;
        }

    }

    if (ferror(stdin)){
        exitProgram(EXIT_FAILURE, "fgets failed");
    }

    puts("strings:");
    printListAndFree(start);

    exitProgram(EXIT_SUCCESS, NULL);
}