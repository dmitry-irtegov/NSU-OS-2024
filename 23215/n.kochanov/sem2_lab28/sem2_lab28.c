#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
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

void parse_url(const char *url, char *host, char *path) {
    if (strncmp(url, "http://", 7) == 0) {
        url += 7;
    }

    const char *slash = strchr(url, '/');
    if (slash) {
        strncpy(host, url, slash - url);
        host[slash - url] = '\0';
        strcpy(path, slash);
    } else {
        strcpy(host, url);
        strcpy(path, "/");
    }
}

int create_socket(const char *host) {
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
    serv_addr.sin_port = htons(PORT);
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
    parse_url(argv[1], host, path);

    int sockfd = create_socket(host);
    if (sockfd < 0) return 1;

    char request[2048];
    snprintf(request, sizeof(request),
             "GET %s HTTP/1.0\r\nHost: %s\r\nConnection: close\r\n\r\n",
             path, host);
    send(sockfd, request, strlen(request), 0);

    fd_set readfds;
    char buffer[BUFFER_SIZE];
    int lines_printed = 0;
    int show_more = 1;

    struct termios original_termios;
    tcgetattr(STDIN_FILENO, &original_termios);
    enable_raw_mode(&original_termios);

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        FD_SET(STDIN_FILENO, &readfds);
        int maxfd = sockfd > STDIN_FILENO ? sockfd : STDIN_FILENO;

        if (select(maxfd + 1, &readfds, NULL, NULL, NULL) < 0) {
            perror("select");
            break;
        }

        if (FD_ISSET(sockfd, &readfds) && show_more) {
            int n = read(sockfd, buffer, sizeof(buffer) - 1);
            if (n <= 0) break;
            buffer[n] = '\0';

            char *line = strtok(buffer, "\n");
            while (line) {
                printf("%s\n", line);
                lines_printed++;

                if (lines_printed >= LINES_PER_PAGE) {
                    printf("\n--- Press space to scroll down ---\n");
                    fflush(stdout);
                    show_more = 0;
                    break;
                }
                line = strtok(NULL, "\n");
            }
        }

        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            char c;
            if (read(STDIN_FILENO, &c, 1) > 0) {
                if (c == ' ') {
                    lines_printed = 0;
                    show_more = 1;
                }
            }
        }
    }

    disable_raw_mode(&original_termios);
    close(sockfd);
    return 0;
}