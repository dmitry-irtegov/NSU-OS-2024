#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_BUF_SIZE 1000

struct Node {
    char *str;
    struct Node *next;
};

struct Node* create_node(char *str) {
    struct Node *new_node = malloc(sizeof(struct Node));
    if (!new_node) {
        perror("malloc");
        return NULL;
    }
    new_node->str = malloc(strlen(str) + 1);
    if (!new_node->str) {
        free(new_node);
        perror("malloc");
        return NULL;
    }
    strcpy(new_node->str, str);
    new_node->next = NULL;
    return new_node;
}

void free_list(struct Node *head) {
    struct Node *tmp;
    while (head != NULL) {
        tmp = head;
        head = head->next;
        free(tmp->str);
        free(tmp);
    }
}

int read_line(char **buffer, size_t* current_buf_size) {
    size_t pos = 0;
    while (1) {
        if (fgets(*buffer + pos, *current_buf_size - pos, stdin) == NULL) {
            if (feof(stdin)) {
                fprintf(stderr, "Contract was broken.\n");
                return -1;
            } else {
                perror("fgets");
                return -1;
            }
        }

        pos += strlen(*buffer + pos);

        if ((*buffer)[pos - 1] == '\n') {
            break;
        }

        if (pos == *current_buf_size - 1) {
            size_t new_size = *current_buf_size * 2;
            char *new_ptr = realloc(*buffer, new_size);
            if (new_ptr == NULL) {
                perror("realloc");
                return -1;
            }
            *buffer = new_ptr;
            *current_buf_size = new_size;
        }
    }

    return 0;
}

int main() {
    char* buffer = (char*)malloc(INITIAL_BUF_SIZE);
    if (buffer == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    size_t current_buf_size = INITIAL_BUF_SIZE;

    struct Node *head = NULL;
    struct Node **tail_ptr = &head;

    while (1) {
        if (read_line(&buffer, &current_buf_size) == -1) {
            free_list(head);
            free(buffer);
            exit(EXIT_FAILURE);
        }
        if (buffer[0] == '.') {
            break;
        }
        struct Node *node = create_node(buffer);
        if (node == NULL) {
            free_list(head);
            free(buffer);
            exit(EXIT_FAILURE);
        }
        *tail_ptr = node;
        tail_ptr = &(node->next);
    }

    struct Node *tmp = head;
    while (tmp != NULL) {
        printf("%s", tmp->str);
        tmp = tmp->next;
    }

    free_list(head);
    free(buffer);
    exit(EXIT_SUCCESS);
}

