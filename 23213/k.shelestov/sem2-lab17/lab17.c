#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define MAXLEN 80

typedef struct list_elem {
    struct list_elem* next;
    char* data;
} list_elem;

typedef struct head_list_elem {
    list_elem* first;
    int size;
} head_list_elem;

pthread_mutex_t global_mutex;

head_list_elem* cont_allocate() {
    head_list_elem* head = (head_list_elem*)malloc(sizeof(head_list_elem));
    assert(head);
    head->size = 0;
    head->first = NULL;
    return head;
}

void cont_free(head_list_elem* root) {
    pthread_mutex_lock(&global_mutex);
    list_elem* cur = root->first;
    list_elem* tmp = NULL;
    while (cur) {
        tmp = cur->next;
        free(cur->data);
        free(cur);
        cur = tmp;
    }
    free(root);
    pthread_mutex_unlock(&global_mutex);    
}

void cont_add_first(head_list_elem* root, char* str) {
    list_elem* newElement = (list_elem*)malloc(sizeof(list_elem));
    assert(newElement);
    newElement->data = str;
    pthread_mutex_lock(&global_mutex);
    newElement->next = root->first;
    root->first = newElement;
    root->size++;
    pthread_mutex_unlock(&global_mutex);
}

void swap(list_elem* a, list_elem* b) {
    char* tmp = a->data;
    a->data = b->data;
    b->data = tmp;
}

void* cont_sort(void* arg) {
    head_list_elem* root = (head_list_elem*)arg;
    while (1) {
        pthread_mutex_lock(&global_mutex);
        list_elem *iteri, *iterj;
        for (iteri = root->first; iteri; iteri = iteri->next) {
            for (iterj = iteri->next; iterj; iterj = iterj->next) {
                if (strcmp(iteri->data, iterj->data) > 0) {
                    swap(iteri, iterj);
                }
            }
        }
        pthread_mutex_unlock(&global_mutex);
        sleep(5); 
    }
    return NULL;
}

void cont_print(head_list_elem* root) {
    pthread_mutex_lock(&global_mutex);
    list_elem* cur = root->first;
    printf("________________________________\n");
    while (cur) {
        printf("%s\n", cur->data);
        cur = cur->next;
    }
    printf("________________________________\n");
    pthread_mutex_unlock(&global_mutex);
}

int main(int argc, char* argv[]) {
    head_list_elem* root = cont_allocate();
    pthread_mutex_init(&global_mutex, NULL);
    pthread_t sort_thread;
    pthread_create(&sort_thread, NULL, cont_sort, (void*)root);
    char* cur = NULL;
    while (1) {
        cur = (char*)calloc(MAXLEN, sizeof(char));
        assert(cur);
        if (fgets(cur, MAXLEN, stdin) == NULL) {
            free(cur);
            break;
        }
        size_t len = strlen(cur);
        if (len > 0 && cur[len - 1] == '\n') {
            cur[len - 1] = '\0';
            len--;
        }
        if (len == 0) {
            cont_print(root); 
            free(cur);
            continue;
        }
        cont_add_first(root, cur); 
    }

    pthread_cancel(sort_thread); 
    pthread_join(sort_thread, NULL);
    cont_free(root);
    pthread_mutex_destroy(&global_mutex);
    return 0;
}

