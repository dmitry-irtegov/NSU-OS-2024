#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#define LEN 80

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct Node {
    char* val;
    struct Node *next;
} Node;
Node dummy = {0};

Node * create(char* val) {
    Node *newNode = (Node*)malloc(sizeof(Node));
    assert(newNode);
    newNode->val = (char *)malloc(sizeof(char)*100);
    assert(newNode->val);
    strcpy(newNode->val, val);
    newNode->next = NULL;
    return newNode;
}
void addAfter(Node *head, char* val) {
    Node* newNode = create(val);
    newNode->next = head->next;
    head->next = newNode;
}

void bubleSort(Node* head) {
    int swapped;
    Node *ptr1;
    Node *rptr = NULL;
    if (head == NULL)
        return;
    do {
        swapped = 0;
        ptr1 = head;
        while (ptr1->next != rptr) {
            if (strcmp(ptr1->val, ptr1->next->val) > 0) {
                char *temp = ptr1->val;
                ptr1->val = ptr1->next->val;
                ptr1->next->val = temp;
                swapped = 1;
            }
            ptr1 = ptr1->next;
        }
        rptr = ptr1;
    } while (swapped);
}
void printList(Node* head) {
    Node *temp = head;
    while (temp != NULL) {
        printf("%s\n", temp->val);
        temp = temp->next;
    }
}

void * child(void * val) {
    while(1) {
        sleep(5);
        pthread_mutex_lock(&mutex);
        bubleSort(dummy.next);
        pthread_mutex_unlock(&mutex);
    }
}

void freeList(Node* head) {
    Node *tmp;
    while (head != NULL) {
        tmp = head;
        head = head->next;
        free(tmp->val);
        free(tmp);
    }
}

int main(int argc, char* argv) {
    pthread_t th;
    int err = 0;
    if ((err = pthread_create(&th, NULL, child, NULL)) != 0) {
        fprintf(stderr, "Couldn`t open thread: %s \n", strerror(err));
        return 1;
    }
    char * buf = (char*)malloc(LEN * sizeof(char));
    assert(buf);
    while(1) {
        printf("Enter text or blank to see status or EOF to close:");
        if(fgets(buf, LEN, stdin) == NULL) {
            pthread_cancel(th); 
            pthread_join(th, NULL);
            freeList(dummy.next);
            pthread_mutex_destroy(&mutex);
            free(buf);
            putc('\n', stdout);
            return 0;
        }
        if(strlen(buf) > 1){
            if (buf[strlen(buf) - 1] == '\n') {
                buf[strlen(buf) - 1] = '\0';
            }
            pthread_mutex_lock(&mutex);
            addAfter(&dummy, buf);
            pthread_mutex_unlock(&mutex);
        } else if(buf[0] == '\n') {
            pthread_mutex_lock(&mutex);
            printList(dummy.next);
            pthread_mutex_unlock(&mutex);
        }
    }
}
