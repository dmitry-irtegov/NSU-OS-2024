#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#define MAXLEN 80
#define TIMEOUT 5
#define BUFSIZE 4096

typedef struct node_t {
    int length;
    char data[MAXLEN];
    struct node_t *next;
    pthread_mutex_t mut;
} Node;

Node* head;

void* sort_loop(void* param) {
    struct node_t *prev, *cur0, *cur1;
    int mess;
    while (1) {
	    sleep(TIMEOUT);
        mess = 1;
        while (mess) {
            mess = 0;
            prev = head;
            pthread_mutex_lock(&(prev->mut));
            cur0 = head->next;
            if (cur0) {
                pthread_mutex_lock(&(cur0->mut));
                cur1 = cur0->next;
                while (cur1) {
                    sleep(1);
                    pthread_mutex_lock(&(cur1->mut));
                    if (strcmp(cur1->data, cur0->data) < 0) {
                        mess = 1;
                        cur0->next = cur1->next;
                        prev->next = cur1;
                        cur1->next = cur0;
                        cur0 = cur1;
                        cur1 = cur0->next;
                    }
                    pthread_mutex_unlock(&(prev->mut));
                    prev = cur0;
                    cur0 = cur1;
                    cur1 = cur1->next;
                }
                pthread_mutex_unlock(&(cur0->mut));
            }
            pthread_mutex_unlock(&(prev->mut));
        }
    }
    return NULL;
}

void add_node(Node* cur, char* data) {
    strncpy(cur->data, data, cur->length);
    cur->data[cur->length] = '\0';
    pthread_mutex_lock(&(head->mut));
    cur->next = head->next;
    head->next = cur;
    pthread_mutex_unlock(&(head->mut));
}

int main() {
    head = (Node*) malloc(sizeof(struct node_t));
    pthread_mutex_init(&(head->mut), 0);
    head->next = NULL;
    head->length = 0;
    char buffer[MAXLEN];
    Node* cur, *prev, *pos;
    pthread_t sorting_thread;
    cur = (Node*) malloc(sizeof(struct node_t));
    pthread_mutex_init(&(cur->mut), 0);
    cur->length = 0;
    pthread_create(&sorting_thread, NULL, sort_loop, NULL);
    while(1) {
        if (fgets(buffer, MAXLEN, stdin) == NULL) {
            exit(1);
        }
        cur->length = strlen(buffer);
        if (cur->length && buffer[cur->length-1] == '\n') {
            buffer[cur->length-1] = '\0';
            cur->length--;
        }
        if (cur->length) {
            add_node(cur, buffer);
            cur = (Node*) malloc(sizeof(struct node_t));
            pthread_mutex_init(&(cur->mut), 0);
            cur->length = 0;
        } else {
            prev = head;
            pthread_mutex_lock(&(prev->mut));
            pos = prev->next;
            while(pos){
                pthread_mutex_lock(&(pos->mut));
                pthread_mutex_unlock(&(prev->mut));
                printf("%s\n", pos->data);
                prev = pos;
                pos = pos->next;
            }
            pthread_mutex_unlock(&(prev->mut));
        }
    }
    return 0;
}
