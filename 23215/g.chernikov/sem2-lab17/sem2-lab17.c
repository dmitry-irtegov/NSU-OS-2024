#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define STRSIZE 80
#define SLEEPTIME 5

typedef struct Node_t {
    char* string;
    struct Node_t* next;
} Node;

Node* head = NULL;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


void insert(char* str) {
    Node* new_node = calloc(1, sizeof(Node));
    new_node->string = str;
    new_node->next = head;
    head = new_node;
}

void bubble() {
    pthread_mutex_lock(&mutex);

    Node *i, *j;
    char* tmp;

    for (i = head; i != NULL; i = i->next) {
        for (j = head; j->next != NULL; j = j->next) {
            if(strcmp(j->string, j->next->string) > 0) {
                tmp = j->string;
                j->string = j->next->string;
                j->next->string = tmp;
            }
        }
    }
    pthread_mutex_unlock(&mutex);
}

void* sorting(void* args) {
    while(1) {
        sleep(SLEEPTIME);
        bubble();
    }
    return NULL;
}

void list_printing() {
    pthread_mutex_lock(&mutex);
    Node* tmp = head;
    while(tmp != NULL) {
        printf("'%s'\n", tmp->string);
        tmp = tmp->next;
    }
    pthread_mutex_unlock(&mutex);
}

int main() {
    pthread_t thread;
    pthread_create(&thread, NULL, sorting, NULL);

    char* buffer = NULL;
    size_t bufsize = 0;

    while(1) {
        ssize_t bytes_read = getline(&buffer, &bufsize, stdin);

        if (buffer[bytes_read - 1] == '\n') {
            buffer[bytes_read - 1] = '\0';
        }
        
        if (strlen(buffer) == 0) {
            list_printing();
            continue;
        }

        pthread_mutex_lock(&mutex);
        char *ptr = buffer;
        while (*ptr) {
            char* str = calloc(STRSIZE + 1, sizeof(char));
            strncpy(str, ptr, STRSIZE);
            insert(str);
            if (strlen(ptr) < STRSIZE) {
                break;
            }
            ptr += STRSIZE;
        }
        pthread_mutex_unlock(&mutex);
    }

    pthread_join(thread, NULL);
    return 0;
}