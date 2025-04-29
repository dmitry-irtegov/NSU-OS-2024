#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define PORT 80
#define BUFFER_SIZE 4096
#define LINES_PER_PAGE 25

void enable_raw_mode(struct termios *original_termios) {
    struct termios raw = *original_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void disable_raw_mode(struct termios *original_termios) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, original_termios);
}

void parse_url(const char *url, char *host, char *path, int *port) {
    *port = PORT;

    if (strncmp(url, "http://", 7) == 0) {
        url += 7;
    }

    const char *slash = strchr(url, '/');
    const char *host_end = slash ? slash : url + strlen(url);

    const char *colon = strchr(url, ':');
    if (colon && colon < host_end) {
        strncpy(host, url, colon - url);
        host[colon - url] = '\0';
        *port = atoi(colon + 1);
    } else {
        strncpy(host, url, host_end - url);
        host[host_end - url] = '\0';
    }

    if (slash) {
        strcpy(path, slash);
    } else {
        strcpy(path, "/");
    }
}

int create_socket(const char *host, int port) {
    struct hostent *server = gethostbyname(host);
    if (!server) {
        fprintf(stderr, "No such host: %s\n", host);
        return -1;
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return -1;
    }

    struct sockaddr_in serv_addr = {0};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect");
        close(sockfd);
        return -1;
    }

    return sockfd;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s http://host/path\n", argv[0]);
        return 1;
    }

    char host[256], path[1024];
    int port;
    parse_url(argv[1], host, path, &port);

    int sockfd = create_socket(host, port);
    if (sockfd < 0) return 1;

    char request[2048];
    snprintf(request, sizeof(request),
             "GET %s HTTP/1.0\r\nHost: %s\r\nConnection: close\r\n\r\n",
             path, host);
    send(sockfd, request, strlen(request), 0);

    size_t total_size = 0;
    size_t capacity = BUFFER_SIZE;
    char *response = malloc(capacity);
    if (!response) {
        fprintf(stderr, "Memory allocation failed\n");
        close(sockfd);
        return 1;
    }

    while (1) {
        if (total_size + BUFFER_SIZE >= capacity) {
            capacity *= 2;
            response = realloc(response, capacity);
            if (!response) {
                fprintf(stderr, "Memory reallocation failed\n");
                close(sockfd);
                return 1;
            }
        }
        ssize_t n = read(sockfd, response + total_size, BUFFER_SIZE);
        if (n <= 0) break;
        total_size += n;
    }
    response[total_size] = '\0';
    close(sockfd);

    char **lines = NULL;
    size_t num_lines = 0;
    char *line = strtok(response, "\n");
    while (line) {
        lines = realloc(lines, sizeof(char*) * (num_lines + 1));
        lines[num_lines++] = line;
        line = strtok(NULL, "\n");
    }

    struct termios original_termios;
    tcgetattr(STDIN_FILENO, &original_termios);
    enable_raw_mode(&original_termios);

    size_t current_line = 0;
    while (current_line < num_lines) {
        size_t lines_to_print = LINES_PER_PAGE;
        while (lines_to_print-- && current_line < num_lines) {
            printf("%s\n", lines[current_line++]);
        }

        if (current_line < num_lines) {
            printf("\n--- Press space to scroll down, any other key to quit ---\n");
            fflush(stdout);

            char c;
            if (read(STDIN_FILENO, &c, 1) <= 0 || c != ' ') break;
        }
    }

    disable_raw_mode(&original_termios);
    free(response);
    free(lines);
    return 0;
}