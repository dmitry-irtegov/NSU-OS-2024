#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>

#define TIMEOUT 5000

typedef struct node_s {
    struct node_s* next;
    struct node_s* prev;
    struct pollfd pfd;
    char* filename;
} node;

typedef struct cycle_s {
    node* head;
    node* current;
} cycle;

void add_file(cycle* cycle, int fd, char* filename) {
    node* new_node = (node*)malloc(sizeof(node));
    if (new_node == NULL){
        perror("malloc failed");
        exit(-1);
    }
    new_node->pfd.fd = fd;
    new_node->pfd.events = POLLIN;
    new_node->filename = filename;
    new_node->next = new_node;
    new_node->prev = new_node;
    if (cycle->head != NULL) {
        new_node->prev = cycle->head->prev;
        cycle->head->prev->next = new_node;
        cycle->head->prev = new_node;
        new_node->next = cycle->head;
    } else {
        cycle->head = new_node;
        cycle->current = new_node;
    }
}

void next(cycle* cycle) {
    if (cycle->current != NULL) {
        cycle->current = cycle->current->next;
    }
}

void remove_file(cycle* cycle) {
    if (cycle->current == NULL) {
        return;
    }
    close(cycle->current->pfd.fd);
    if (cycle->current->next == cycle->current) {
        cycle->head = NULL;
        free(cycle->current);
        cycle->current = NULL;
    } else {
        node* tmp = cycle->current;
        cycle->current->prev->next = cycle->current->next;
        cycle->current->next->prev = cycle->current->prev;
        if (cycle->current == cycle->head) {
            cycle->head = cycle->current->next;
        }
        cycle->current = cycle->current->prev;
        free(tmp);
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "not enough arguments\n");
        exit(-1);
    }

    cycle cycle;
    cycle.current = NULL;
    cycle.head = NULL;

    for (int i = 1; i < argc; i++) {
        int fd = open(argv[i], O_RDONLY);
        if (fd == -1) {
            perror("open failed");
            exit(-1);
        }
        add_file(&cycle, fd, argv[i]);
    }
    
    char buf[BUFSIZ];
    while (cycle.head != NULL) {
        printf("[%s]\n", cycle.current->filename);

        int ret = poll(&cycle.current->pfd, 1, TIMEOUT);
        switch (ret) {
            case -1:
                perror("poll failed");
                exit(-1);
            case 0:
                printf("[%s] timeout\n\n", cycle.current->filename);
                break;
            default:
                if (cycle.current->pfd.revents & POLLIN) {
                    ssize_t bytes_read = read(cycle.current->pfd.fd, buf, sizeof(buf) - 1);
                    if (bytes_read > 0) {
                        buf[bytes_read] = 0;
                        printf("from [%s] readed: %s\n", cycle.current->filename, buf);
                    } else {
                        if (bytes_read != 0) {
                            perror("read failed");
                            exit(-1);
                        }
                        printf("end of file: [%s] closed\n\n", cycle.current->filename);
                        remove_file(&cycle);
                    }
                }
                break;
        }
        next(&cycle);
    }
    exit(0);
}
