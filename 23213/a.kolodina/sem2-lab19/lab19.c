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
    int flag;
    while (1) {
        sleep(TIMEOUT);
        mess = 1;
        while (mess) {
            mess = 0;
            prev = head;
            while (1) {
                flag = 0;
                pthread_mutex_lock(&(prev->mut));
                cur0 = prev->next;
                if (!cur0) {
                    pthread_mutex_unlock(&(prev->mut));
                    break;
                }
                pthread_mutex_lock(&(cur0->mut));
                cur1 = cur0->next;
                if (!cur1) {
                    pthread_mutex_unlock(&(cur0->mut));
                    pthread_mutex_unlock(&(prev->mut));
                    break;
                }
                pthread_mutex_lock(&(cur1->mut));
                if (strcmp(cur1->data, cur0->data) < 0) {
                    mess = 1;
                    flag = 1;
                    cur0->next = cur1->next;
                    prev->next = cur1;
                    cur1->next = cur0;
                }
                pthread_mutex_unlock(&(cur1->mut));
                pthread_mutex_unlock(&(cur0->mut));
                pthread_mutex_unlock(&(prev->mut));
                if (flag) {
                    sleep(1); 
                    prev = cur1; 
                } else {
                    prev = cur0;
                }
            }
        }
    }
    return NULL;
}

int main() {
    int code;
    head = (Node*) malloc(sizeof(struct node_t));
    if (head == NULL) {
        fprintf(stderr, "malloc error");
        exit(1);
    }
    code = pthread_mutex_init(&(head->mut), NULL);
    if (code != 0) {
        fprintf(stderr, "pthread_mutex_init error %s\n", strerror(code));
        exit(1);
    }
    head->next = NULL;
    head->length = 0;
    char buffer[MAXLEN];
    Node *cur, *prev, *pos;
    pthread_t sorting_thread;
    cur = (Node*) malloc(sizeof(struct node_t));
    if (cur == NULL) {
        fprintf(stderr, "malloc error");
        exit(1);
    }
    code = pthread_mutex_init(&(cur->mut), NULL);
    if (code != 0) {
        fprintf(stderr, "pthread_mutex_init error %s\n", strerror(code));
        exit(1);
    }
    cur->length = 0;
    cur->next = NULL;
    code = pthread_create(&sorting_thread, NULL, sort_loop, NULL);
    if (code != 0) {
        fprintf(stderr, "pthread_create error %s\n", strerror(code));
        exit(1);
    }
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
            strncpy(cur->data, buffer, cur->length);
            cur->data[cur->length] = '\0';
            pthread_mutex_lock(&(head->mut));
            cur->next = head->next;
            head->next = cur;
            pthread_mutex_unlock(&(head->mut));
            cur = (Node*) malloc(sizeof(struct node_t));
            if (cur == NULL) {
                fprintf(stderr, "malloc error");
                exit(1);
            }
            code = pthread_mutex_init(&(cur->mut), NULL);
            if (code != 0) {
                fprintf(stderr, "pthread_mutex_init error %s\n", strerror(code));
                exit(1);
            }
            cur->length = 0;
            cur->next = NULL;
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
