#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <ctype.h>

#define BUFFER_SIZE 4096
#define LINES_PER_PAGE 25

int parse_url(const char *url, char *host, char *path) {
    const char *p;
    if (strncmp(url, "http://", 7) == 0) {
        p = url + 7;
    } else {
        p = url;
    }

    const char *slash = strchr(p, '/');
    if (slash) {
        strncpy(host, p, slash - p);
        host[slash - p] = '\0';
        strcpy(path, slash);
    } else {
        strcpy(host, p);
        strcpy(path, "/");
    }

    return 0;
}

int connect_to_host(const char *host) {
    struct addrinfo hints, *res, *rp;
    int sockfd;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(host, "80", &hints, &res) != 0) {
        perror("getaddrinfo");
        return -1;
    }

    for (rp = res; rp != NULL; rp = rp->ai_next) {
        sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sockfd == -1) continue;
        if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) == 0) break;
        close(sockfd);
    }

    freeaddrinfo(res);
    if (rp == NULL) return -1;
    return sockfd;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        const char *usage = "Usage: ./client http://host/path\n";
        write(STDERR_FILENO, usage, strlen(usage));
        return 1;
    }

    char host[256], path[1024];
    parse_url(argv[1], host, path);
    int sockfd = connect_to_host(host);
    if (sockfd < 0) {
        perror("connect");
        return 1;
    }

    char request[2048];
    snprintf(request, sizeof(request),
             "GET %s HTTP/1.0\r\nHost: %s\r\nConnection: close\r\n\r\n",
             path, host);
    write(sockfd, request, strlen(request));

    fd_set readfds;
    char buffer[BUFFER_SIZE];
    int lines_printed = 0;
    int in_body = 0;

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        FD_SET(STDIN_FILENO, &readfds);
        int maxfd = sockfd > STDIN_FILENO ? sockfd : STDIN_FILENO;

        if (select(maxfd + 1, &readfds, NULL, NULL, NULL) < 0) {
            perror("select");
            break;
        }

        if (FD_ISSET(sockfd, &readfds)) {
            ssize_t n = read(sockfd, buffer, sizeof(buffer) - 1);
            if (n <= 0) break;
            buffer[n] = '\0';

            char *line = strtok(buffer, "\n");
            while (line) {
                if (!in_body) {
                    if (strcmp(line, "\r") == 0 || strcmp(line, "") == 0) {
                        in_body = 1;
                    }
                } else {
                    size_t len = strlen(line);
                    write(STDOUT_FILENO, line, len);
                    write(STDOUT_FILENO, "\n", 1);
                    lines_printed++;
                    if (lines_printed >= LINES_PER_PAGE) {
                        const char *prompt = "\nPress space to scroll down\n";
                        write(STDOUT_FILENO, prompt, strlen(prompt));
                        while (1) {
                            FD_ZERO(&readfds);
                            FD_SET(STDIN_FILENO, &readfds);
                            select(STDIN_FILENO + 1, &readfds, NULL, NULL, NULL);
                            if (FD_ISSET(STDIN_FILENO, &readfds)) {
                                char c;
                                read(STDIN_FILENO, &c, 1);
                                if (c == ' ') break;
                            }
                        }
                        lines_printed = 0;
                    }
                }
                line = strtok(NULL, "\n");
            }
        }
    }

    close(sockfd);
    return 0;
}
