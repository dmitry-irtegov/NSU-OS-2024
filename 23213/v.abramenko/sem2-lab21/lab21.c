#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#define MAXLEN 80
#define TIMEOUT 5
#define DELAY 1

typedef struct node { 
    int length;
    char data[MAXLEN];
    struct node *next;
    pthread_rwlock_t rwlock;
} node_t;

node_t* head;

void* sort_loop(void* param) {
    node_t *prev, *cur0, *cur1, *next;
    while (1) {
        sleep(TIMEOUT);
        int sorted = 0;
        int flag;
        while (!sorted) {
            sorted = 1;
            prev = head;
            while (1) {
                flag = 0;
                pthread_rwlock_rdlock(&(prev->rwlock));
                cur0 = prev->next;
                if (cur0 == NULL) {
                    pthread_rwlock_unlock(&(prev->rwlock));
                    break;
                }
                pthread_rwlock_rdlock(&(cur0->rwlock));
                next = cur0;
                cur1 = cur0->next;
                if (cur1 == NULL) {
                    pthread_rwlock_unlock(&(cur0->rwlock));
                    pthread_rwlock_unlock(&(prev->rwlock));
                    break;
                }
                pthread_rwlock_rdlock(&(cur1->rwlock));
                if (strcmp(cur1->data, cur0->data) < 0) {
                    flag = 1;
                    pthread_rwlock_wrlock(&(prev->rwlock));
                    pthread_rwlock_wrlock(&(cur0->rwlock));
                    pthread_rwlock_wrlock(&(cur1->rwlock));
                    cur0->next = cur1->next;
                    prev->next = cur1;
                    cur1->next = cur0;
                    next = cur1;
                    sorted = 0;
                }
                pthread_rwlock_unlock(&(cur1->rwlock));
                pthread_rwlock_unlock(&(cur0->rwlock));
                pthread_rwlock_unlock(&(prev->rwlock));
                if (flag) {
                    sleep(DELAY); 
                }
                prev = next;
            }
        }
    }
    return NULL;
}

int main() {
    int code;
    head = (node_t*) malloc(sizeof(node_t));
    if (head == NULL) {
        perror("malloc error");
        exit(EXIT_FAILURE);
    }
    if ((code = pthread_rwlock_init(&(head->rwlock), NULL)) != 0) {
        fprintf(stderr, "pthread_rwlock_init error %s\n", strerror(code));
        exit(EXIT_FAILURE);
    }
    head->next = NULL;
    head->length = 0;
    char buffer[MAXLEN];
    node_t *cur, *prev, *pos;
    pthread_t sorting_thread;
    if ((code = pthread_create(&sorting_thread, NULL, sort_loop, NULL)) != 0) {
        fprintf(stderr, "pthread_create error %s\n", strerror(code));
        exit(EXIT_FAILURE);
    }
    while(1) {
        if (fgets(buffer, MAXLEN, stdin) == NULL) {
            exit(EXIT_FAILURE);
        }
        int len = strlen(buffer);
        if (len && buffer[len-1] == '\n') {
            buffer[len-1] = '\0';
            len--;
        }
        if (len) {
            cur = (node_t*) malloc(sizeof(node_t));
            if (cur == NULL) {
                perror("malloc error");
                exit(EXIT_FAILURE);
            }
            if ((code = pthread_rwlock_init(&(cur->rwlock), NULL)) != 0) {
                fprintf(stderr, "pthread_rwlock_init error %s\n", strerror(code));
                exit(EXIT_FAILURE);
            }
            cur->length = len;
            cur->next = NULL;
            strncpy(cur->data, buffer, cur->length);
            cur->data[cur->length] = '\0';
            pthread_rwlock_wrlock(&(head->rwlock));
            cur->next = head->next;
            head->next = cur;
            pthread_rwlock_unlock(&(head->rwlock));
        } else {
            prev = head;
            pthread_rwlock_rdlock(&(prev->rwlock));
            pos = prev->next;
            while(pos) {
                pthread_rwlock_rdlock(&(pos->rwlock));
                pthread_rwlock_unlock(&(prev->rwlock));
                printf("%s\n", pos->data);
                prev = pos;
                pos = pos->next;
            }
            pthread_rwlock_unlock(&(prev->rwlock));
        }
    }
    return 0; 
}
