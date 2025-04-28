#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <termios.h>
#include <sys/select.h>

void parse_url(const char *url, char *host, char *path) {
    if (strncmp(url, "http://", 7) == 0) {
        url += 7;
    }
    const char *slash = strchr(url, '/');
    strncpy(host, url, slash - url);
    host[slash - url] = '\0';
    strcpy(path, slash);
}

void echo_nonbuff_mode(int enable) {
    static struct termios old_t;
    struct termios new_t;
    if (enable) {
        tcgetattr(0, &old_t);
        new_t = old_t;
        new_t.c_lflag &= ~(ICANON | ECHO);
        new_t.c_cc[VMIN]=1;
        tcsetattr(0, TCSANOW, &new_t);
    } else {
        tcsetattr(0, TCSANOW, &old_t);
    }
}

int main(int argc, char *argv[]) {
    char host[256], path[1024], request[2048], buffer[4096];
    struct addrinfo hints, *res;
    int sockfd;
    int new_line_cnt = 0;
    int paused = 0;
    parse_url(argv[1], host, path);
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(host, "80", &hints, &res) != 0) {
        perror("getaddrinfo");
        exit(1);
    }
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0) {
        perror("socket");
        exit(2);
    }
    if (connect(sockfd, res->ai_addr, res->ai_addrlen) != 0) {
        perror("connect");
        exit(3);
    }
    freeaddrinfo(res);
    snprintf(request, sizeof(request), "GET %s HTTP/1.0\r\nHost: %s\r\n\r\n", path, host);
    send(sockfd, request, strlen(request), 0);
    echo_nonbuff_mode(1);
    fd_set readfds;
    int bytes_received;
    while (1) {
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        if (paused) {
            FD_SET(0, &readfds);
        }
        if (select(sockfd + 1, &readfds, NULL, NULL, NULL) < 0) {
            perror("select");
            exit(4);
        }
        if (FD_ISSET(sockfd, &readfds) && !paused) {
            bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
            if (bytes_received <= 0) {
                break;
            }
            buffer[bytes_received] = '\0';
            char *data = buffer;
            for (char *p = data; *p; ++p) {
                putchar(*p);
                if (*p == '\n') {
                    new_line_cnt++;
                    if (new_line_cnt >= 25) {
                        printf("\nPress space to scroll down...");
                        fflush(stdout);
                        paused = 1;
                        break;
                    }
                }
            }
            fflush(stdout);
        }
        if (FD_ISSET(0, &readfds) && paused) {
            char c;
            if (read(0, &c, 1) > 0) {
                if (c == ' ') {
                    new_line_cnt = 0;
                    paused = 0;
                }
            }
        }
    }
    close(sockfd);
    echo_nonbuff_mode(0);
    exit(0);
}