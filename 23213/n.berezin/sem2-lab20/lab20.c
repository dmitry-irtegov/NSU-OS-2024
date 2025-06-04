#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#define LINE_LEN 80

struct node {
    char *data;
    struct node *next;
    pthread_rwlock_t lock;
};

/// Special node in the list that doesn't store data
struct node head = {
    .data = NULL,
    .next = NULL,
    .lock = PTHREAD_RWLOCK_INITIALIZER
};

bool should_swap(struct node *lower, struct node *greater) {
    return strcmp(lower->data, greater->data) > 0;
}

void list_sort() {
    bool sorted = false;

    while (!sorted) {
        sorted = true;

        struct node *current = &head;

        while (1) {
            pthread_rwlock_wrlock(&current->lock);

            struct node *a = current->next;
            if (a == NULL) {
                pthread_rwlock_unlock(&current->lock);
                break;
            }

            pthread_rwlock_wrlock(&a->lock);

            struct node *b = a->next;
            if (b == NULL) {
                pthread_rwlock_unlock(&current->lock);
                pthread_rwlock_unlock(&a->lock);
                break;
            }

            pthread_rwlock_wrlock(&b->lock);

            struct node *after_current = a;
            if (should_swap(a, b)) {
                current->next = b;
                a->next = b->next;
                b->next = a;
                sorted = false;
                after_current = b;
            }
            
            pthread_rwlock_unlock(&current->lock);
            pthread_rwlock_unlock(&a->lock);
            pthread_rwlock_unlock(&b->lock);

            current = after_current;
        }
    }
}

void *sorting_thread(void *arg) {
    while (1) {
        sleep(5);
        list_sort();
        fprintf(stderr, "Sorted!\n");
    }
}

void print_list() {
    printf("List contents:\n");

    struct node *current = &head;
    pthread_rwlock_rdlock(&head.lock);
    
    while (current->next != NULL) {
        struct node *next = current->next;
        pthread_rwlock_rdlock(&next->lock);
        pthread_rwlock_unlock(&current->lock);

        printf("  - %s\n", next->data);

        current = next;
    }

    pthread_rwlock_unlock(&current->lock);
}

int main(int argc, char *argv[]) {
    pthread_t tid;
    int err = pthread_create(&tid, NULL, sorting_thread, NULL);
    if (err != 0) {
        perror("Failed to create thread");
        exit(1);
    }

    char buf[LINE_LEN];

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

            struct node *new_head = malloc(sizeof(struct node));
            if (new_head == NULL) {
                perror("Not enough memory");
                return 1;
            }
            new_head->data = malloc(strlen(buf) + 1);
            if (new_head->data == NULL) {
                perror("Not enough memory");
                return 1;
            }
            strcpy(new_head->data, buf);

            int err = pthread_rwlock_init(&new_head->lock, NULL);
            if (err != 0) {
                fprintf(stderr, "Failed to initialize mutex: %s\n", strerror(err));
                free(new_head);
                continue;
            }

            pthread_rwlock_wrlock(&head.lock);
            new_head->next = head.next;
            head.next = new_head;
            pthread_rwlock_unlock(&head.lock);
        }
    }
}
