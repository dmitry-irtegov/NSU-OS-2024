#define _REENTRANT
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <thread.h>
#include <unistd.h>
#include <string.h>

#define MAXSTR 80

typedef struct elem_s {
    char str[MAXSTR + 1];
    struct elem_s* next;
    struct elem_s* prev;
} elem_t;

typedef struct list_s {
    pthread_mutex_t mutex;
    int listlen;
    elem_t* first;
    elem_t* last;
} list_t;

void list_init(list_t* list)
{
    pthread_mutex_init(&(list->mutex), NULL);
    list->listlen = 0;
    list->first = NULL;
    list->last = NULL;
}

void list_add(list_t* list, char* str)
{
    elem_t* cur = (elem_t*)malloc(sizeof(elem_t));
    pthread_mutex_lock(&(list->mutex));
    if(list->first == NULL) {
        list->first = cur;
        list->last = cur;
        cur->prev = NULL;
        cur->next = NULL;
    } else {
        cur->prev = NULL;
        cur->next = list->first;
        list->first->prev = cur;
        list->first = cur;
    }
    strncpy(cur->str, str, MAXSTR);
    list->listlen++;
    pthread_mutex_unlock(&(list->mutex));
}

void list_swap(list_t* list, elem_t* ael, elem_t* bel)
{
    if(ael == NULL || bel == NULL || ael == bel) {
        return;
    }
    if(ael->next != bel) {
        return;
    }

    elem_t* a_prev = ael->prev;
    elem_t* b_next = bel->next;

    ael->next = b_next;
    bel->prev = a_prev;
    bel->next = ael;
    ael->prev = bel;

    if(b_next) 
        b_next->prev = ael;
    if(a_prev) 
        a_prev->next = bel;
    

    if(list->first == ael) 
        list->first = bel;
    if(list->last == bel) 
        list->last = ael;

    if(ael->prev)
        ael->prev->next = ael;
    else
        list->first = ael; 
    

    if(ael->next)
        ael->next->prev = ael;
    else
        list->last = ael;

    if(bel->prev)
        bel->prev->next = bel;
    else
        list->first = bel;
    

    if(bel->next)
        bel->next->prev = bel;
    else
        list->last = bel;
}

void list_sort(list_t* list)
{
    pthread_mutex_lock(&(list->mutex));
    for(int i = 0; i < list->listlen; i++) {
        for(elem_t* iter = list->first; iter != NULL; iter = iter->next) {
            if(iter->next != NULL && strcmp(iter->str, iter->next->str) > 0){
                list_swap(list, iter, iter->next);
            }
        }
    }
    pthread_mutex_unlock(&(list->mutex));
}

void list_print(list_t list)
{
    pthread_mutex_lock(&(list.mutex));
    elem_t* cur = list.first;
    while(cur) {
        printf("%s\n", cur->str);
        cur = cur->next;
    }
    pthread_mutex_unlock(&(list.mutex));
}

void* sort_thread(void* arg) 
{
    list_t *list = (list_t*)arg;
    while(1) {
        sleep(5);
        list_sort(list);
    }
    return NULL;
}

int main(int argc, char** argv) {
    list_t list;
    list_init(&list);

    pthread_t thread;
    pthread_create(&thread, NULL, sort_thread, (void*)&list);

    char buffer[MAXSTR];
    while(1) {
        char* s = fgets(buffer, sizeof(buffer), stdin);
        if(s != NULL) {
            if(buffer[0] == '\n') {
                list_print(list);
                continue;
            }
            size_t len = strcspn(buffer, "\n");
            if(len < sizeof(buffer)) {
                buffer[len] = '\0';
            }
            list_add(&list, buffer);
        } else {
            fprintf(stderr, "unable to read");
            return 1;
        }
    }
    return 0;
}

