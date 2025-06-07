#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <termios.h>

#define PORT 80

typedef struct dynamic_array_t {
    char *buffer;
    size_t size;
    size_t capacity;
} dynamic_array;

void parse_url(char *url, char **host, char **path) {
    if (strncmp(url, "http://", 7) != 0) {
        fprintf(stderr, "bad url\n");
        exit(EXIT_FAILURE);
    }

    url += 7;

    if (*url == 0) {
        fprintf(stderr, "no host\n");
        exit(EXIT_FAILURE);
    }

    char *sl = strchr(url, '/');

    if (sl) {
        *sl = '\0';
        *host = strdup(url);
        *sl = '/';
        *path = strdup(sl);
    } else {
        *host = strdup(url);
        *path = (char *)malloc(2);
        (*path)[0] = '/';
        (*path)[1] = '\0';
    }
}

char stopped = 0;
int lines = 0;
char all_printed = 0;
size_t read_pos = 0;
char no_more_data = 0;

char *curr_line = NULL;
size_t index_in_curr_line = 0;
size_t columns = 0;

void print_line() {
    if (lines == 0) {
        printf("\r                                        ");
        printf("\r%s\n", curr_line);
    } else {
        printf("%s\n", curr_line);
    }
}

void print_page(dynamic_array *dn_array) {
    while (read_pos < dn_array->size) {

        if (index_in_curr_line < columns) {
            curr_line[index_in_curr_line] = dn_array->buffer[read_pos];
            index_in_curr_line++;
            read_pos++;
        } else {
            curr_line[index_in_curr_line] = '\0';
            print_line();
            index_in_curr_line = 0;
            curr_line[index_in_curr_line] = '\0';
            lines++;

            if (lines == 25) {
                lines = 0;
                stopped = 1;
                printf("----- Press space to scroll down -----");
                break;
            }

            continue;
        }

        if (dn_array->buffer[read_pos - 1] == '\t') {
            curr_line[index_in_curr_line - 1] = ' ';

            int i;
            
            for (i = 0; i < 3; i++) {
                if (index_in_curr_line == columns) {
                    curr_line[index_in_curr_line] = '\0';
                    print_line();
                    index_in_curr_line = 0;
                    curr_line[index_in_curr_line] = '\0';
                    lines++;
                    break;
                }

                curr_line[index_in_curr_line] = ' ';
                index_in_curr_line++;
            }

            for (int j = 0; j < 3 - i; j++) {
                curr_line[index_in_curr_line] = ' ';
                index_in_curr_line++;
            }
        }
        else if (dn_array->buffer[read_pos - 1] == '\n') {
            curr_line[index_in_curr_line - 1] = '\0';
            print_line();
            index_in_curr_line = 0;
            curr_line[index_in_curr_line] = '\0';
            lines++;
        }

        if (lines == 25) {
            lines = 0;
            stopped = 1;
            printf("----- Press space to scroll down -----");
            break;
        }
    }

    if (read_pos == dn_array->size && no_more_data) {
        if (curr_line[0]) {
            curr_line[index_in_curr_line] = '\0';
            print_line();
        }
        all_printed = 1;
    }

    fflush(stdout);
}

int main(int argc, char *argv[]) {

    if (argc < 2) {
        fprintf(stderr, "usage: %s <url>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *url = argv[1];
    char *path = NULL;
    char *host = NULL;

    parse_url(url, &host, &path);

    struct hostent *serv = gethostbyname(host);
    if (serv == NULL) {
        herror("gethostbyname");
        free(path);
        free(host);
        exit(EXIT_FAILURE);
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        free(path);
        free(host);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr = *(struct in_addr *)serv->h_addr_list[0];

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sockfd);
        free(path);
        free(host);
        exit(EXIT_FAILURE);
    }

    char request[4096];
    snprintf(request, sizeof(request), "GET %s HTTP/1.0\r\nHost: %s\r\n\r\n", path, host);
    if (send(sockfd, request, strlen(request), 0) < 0) {
        perror("send");
        close(sockfd);
        free(path);
        free(host);
        exit(EXIT_FAILURE);
    }

    fd_set read_fds;

    struct termios orig_term, new_term;
    tcgetattr(0, &orig_term);
    new_term = orig_term;
    new_term.c_lflag &= ~(ICANON | ECHO);
    new_term.c_cc[VMIN] = 1;
    tcsetattr(0, TCSANOW, &new_term);

    dynamic_array dn_array;
    dn_array.buffer = (char *)malloc(BUFSIZ);
    dn_array.size = 0;
    dn_array.capacity = BUFSIZ;

    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    columns = w.ws_col;
    curr_line = (char *)malloc(columns + 1);

    while (1) {

        if (no_more_data && all_printed) {
            break;
        }

        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);
        FD_SET(0, &read_fds);

        if (select(sockfd + 1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            close(sockfd);
            tcsetattr(0, TCSANOW, &orig_term);
            free(path);
            free(host);
            exit(EXIT_FAILURE);
        }

        if (!no_more_data && FD_ISSET(sockfd, &read_fds)) {

            ssize_t bytes_received = recv(sockfd, dn_array.buffer + dn_array.size, dn_array.capacity - dn_array.size, 0);

            if (bytes_received < 0) {
                perror("recv");
                close(sockfd);
                tcsetattr(0, TCSANOW, &orig_term);
                free(path);
                free(host);
                exit(EXIT_FAILURE);
            } else if (bytes_received == 0) {
                no_more_data = 1;
            }

            dn_array.size += bytes_received;

            if (dn_array.capacity == dn_array.size) {
                dn_array.capacity *= 2;
                dn_array.buffer = (char *)realloc(dn_array.buffer, dn_array.capacity);
            }
        }

        if (stopped && FD_ISSET(0, &read_fds)) {
            if (getchar() == ' ') {
                stopped = 0;
            }
        }

        if (!(stopped || all_printed)) {
            print_page(&dn_array);
        }
    }

    close(sockfd);
    tcsetattr(0, TCSANOW, &orig_term);
    free(path);
    free(host);
    exit(EXIT_SUCCESS);
}
