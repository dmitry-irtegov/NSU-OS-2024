#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>

#define TIMEOUT 1000
#define MAX_TIMEOUTS 5

typedef struct node_s {
    struct node_s* next;
    struct node_s* prev;
    struct pollfd pfd;
    char* filename;
    int timeouts;
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
    new_node->timeouts = 0;
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

int readline(char* buf, int len, int fd) {
    char c;
    int i = 0;

    while(i < len && read(fd, &c, 1) > 0) {
        buf[i++] = c;
        if (c == '\n') {
            break;
        }
    }
    buf[i] = 0;
    return i;
}

int main(int argc, char** argv) {
    if (argc < 2){
        fprintf(stderr, "not enough arguments\n");
        exit(-1);
    }

    cycle cycle;
    cycle.current = NULL;
    cycle.head = NULL;

    for (int i = 1; i < argc; i++)
    {
        int fd = open(argv[i], O_RDONLY);
        if (fd == -1) {
            perror("open failed");
            exit(-1);
        }
        add_file(&cycle, fd, argv[i]);
    }
    
    char buf[BUFSIZ];
    while (1) {
        if (cycle.head == NULL) {
            break;
        }

        printf("reading \'%s\'\n", cycle.current->filename);
        
        int ret = poll(&cycle.current->pfd, 1, TIMEOUT);
        switch (ret) {
            case -1:
                perror("poll failed");
                exit(-1);
            case 0:
                cycle.current->timeouts++;
                printf("\n");
                break;
            default:
                cycle.current->timeouts = 0;
                if (cycle.current->pfd.revents & POLLIN) {
                    int bytes_read = readline(buf, BUFSIZ, cycle.current->pfd.fd);
                    if (bytes_read > 0) {
                        printf("%s\n", buf);
                    } else {
                        printf("\'%s\' ended\n", cycle.current->filename);
                        close(cycle.current->pfd.fd);
                        remove_file(&cycle);
                    }
                }
                break;
        }
        if (cycle.current != NULL && cycle.current->timeouts == MAX_TIMEOUTS) {
            printf("closed \'%s\' due to timeouts\n", cycle.current->filename);
            close(cycle.current->pfd.fd);
            remove_file(&cycle);
        }
        next(&cycle);
    }
    exit(0);
}
