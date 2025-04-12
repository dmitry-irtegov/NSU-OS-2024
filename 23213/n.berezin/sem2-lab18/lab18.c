#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <assert.h>

#define LINE_LEN 80

struct node {
    char *data;
    struct node *next;
    pthread_mutex_t lock;
};

struct node *head = NULL;

void node_swap(struct node *a, struct node *b) {
    char *tmp = a->data;
    a->data = b->data;
    b->data = tmp;
}

bool should_swap(struct node *lower, struct node *greater) {
    return strcmp(lower->data, greater->data) > 0;
}

void list_sort() {
    if (head == NULL) {
        return;
    }

    bool sorted = false;

    while (!sorted) {
        sorted = true;

        struct node *current = head;
        
        while (1) {
            struct node *next = current->next;
            if (next == NULL) {
                break;
            }

            int err = pthread_mutex_lock(&current->lock);
            assert(err == 0);
            err = pthread_mutex_lock(&next->lock);
            assert(err == 0);

            if (should_swap(current, next)) {
                node_swap(current, next);
                sorted = false;
            }
            
            err = pthread_mutex_unlock(&current->lock);
            assert(err == 0);
            err = pthread_mutex_unlock(&next->lock);
            assert(err == 0);

            current = next;
        }
    }
}

struct node *new_node(char *data, struct node *next) {
    struct node *res = malloc(sizeof(struct node));
    if (res == NULL) {
        return NULL;
    }
    res->data = data;
    res->next = next;

    int err = pthread_mutex_init(&res->lock, NULL);
    if (err != 0) {
        free(res->data);
        free(res);
        return NULL;
    }


    return res;
}

void *sorting_thread(void *arg) {
    while (1) {
        sleep(5);
        list_sort();
        fprintf(stderr, "Sorted!\n");
    }
}

void print_list() {
    printf("List content:\n");

    struct node *current = head;
    while (current != NULL) {
        printf("  - ");

        int err = pthread_mutex_lock(&current->lock);
        assert(err == 0);

        printf("%s", current->data);
        putchar('\n');
                
        err = pthread_mutex_unlock(&current->lock);
        assert(err == 0);

        current = current->next;
    }
}

int main(int argc, char *argv[]) {
    pthread_t tid;
    int err = pthread_create(&tid, NULL, sorting_thread, NULL);
    if (err != 0) {
        fprintf(stderr, "Failed to create thread: %s\n", strerror(err));
        return 1;
    }

    char *buf = malloc(LINE_LEN);
    if (buf == NULL) {
        perror("Not enough memory");
        return 1;
    }

    char last_char = '\n';
    while (fgets(buf, LINE_LEN, stdin) != NULL) {
        if (last_char == '\n' && buf[0] == '\n') {
            print_list();
        } else {
            size_t len = strlen(buf);
            if (len == 0) {
                continue;
            }

            last_char = buf[len - 1];
            if (buf[len - 1] == '\n') {
                buf[len - 1] = '\0';
            }

            struct node *new_head = new_node(buf, head);
            if (new_head == NULL) {
                fprintf(stderr, "Error creating list head");
                return 1;
            }

            head = new_head;

            buf = malloc(LINE_LEN);
            if (buf == NULL) {
                perror("Not enough memory");
                return 1;
            }
        }
    }

    return 0;
}
