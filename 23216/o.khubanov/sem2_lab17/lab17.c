#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_LENGTH 80

// Узел списка
typedef struct Node {
    char *data;
    struct Node *next;
} Node;

Node *head = NULL;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Функция добавления строки в начало списка
void add_to_list(const char *str) {
    Node *new_node = (Node *)malloc(sizeof(Node));
    if (!new_node) {
        perror("Ошибка выделения памяти");
        exit(EXIT_FAILURE);
    }
    new_node->data = strdup(str);
    new_node->next = NULL;

    pthread_mutex_lock(&mutex);
    new_node->next = head;
    head = new_node;
    pthread_mutex_unlock(&mutex);
}

// Функция разбиения длинных строк и добавления в список
void split_and_add(const char *input) {
    size_t len = strlen(input);
    for (size_t i = 0; i < len; i += MAX_LENGTH) {
        char temp[MAX_LENGTH + 1];
        strncpy(temp, input + i, MAX_LENGTH);
        temp[MAX_LENGTH] = '\0';
        add_to_list(temp);
    }
}

// Функция вывода списка
void print_list() {
    pthread_mutex_lock(&mutex);
    Node *current = head;
    printf("Текущее состояние списка:\n");
    while (current) {
        printf("%s\n", current->data);
        current = current->next;
    }
    printf("-----\n");
    pthread_mutex_unlock(&mutex);
}

// Функция пузырьковой сортировки списка
void bubble_sort() {
    if (!head) return;
    
    int swapped;
    Node *ptr;
    Node *lptr = NULL;

    do {
        swapped = 0;
        ptr = head;

        while (ptr->next != lptr) {
            if (strcmp(ptr->data, ptr->next->data) > 0) {
                char *temp = ptr->data;
                ptr->data = ptr->next->data;
                ptr->next->data = temp;
                swapped = 1;
            }
            ptr = ptr->next;
        }
        lptr = ptr;
    } while (swapped);
}

// Функция работы потока сортировки
void *sort_thread(void *arg) {
    while (1) {
        sleep(5);
        pthread_mutex_lock(&mutex);
        bubble_sort();
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main() {
    pthread_t tid;
    pthread_create(&tid, NULL, sort_thread, NULL);

    char input[256];
    while (1) {
        printf("Введите строку: ");
        if (!fgets(input, sizeof(input), stdin)) {
            perror("Ошибка ввода");
            break;
        }

        // Убираем перевод строки
        size_t len = strlen(input);
        if (len > 0 && input[len - 1] == '\n') {
            input[len - 1] = '\0';
        }

        if (strlen(input) == 0) {
            print_list();
        } else {
            split_and_add(input);
        }
    }

    pthread_cancel(tid);
    pthread_join(tid, NULL);
    pthread_mutex_destroy(&mutex);

    return 0;
}

