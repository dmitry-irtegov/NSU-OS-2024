#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <termios.h>

#define BUF_SIZE 4096
#define LINES_PER_PAGE 25

void set_raw_mode(struct termios *orig_term) {
    struct termios raw = *orig_term;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

int connect_to_host(const char *host, const char *port) {
    struct addrinfo hints = {0}, *res;
    int sock;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    getaddrinfo(host, port, &hints, &res);
    sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    connect(sock, res->ai_addr, res->ai_addrlen);
    freeaddrinfo(res);
    return sock;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s http://host/path\n", argv[0]);
        return 1;
    }

    char host[256], path[1024] = "/";
    if (sscanf(argv[1], "http://%255[^/]%1023s", host, path + 1) < 1) {
        fprintf(stderr, "Invalid URL format\n");
        return 1;
    }

    int sock = connect_to_host(host, "80");

    char request[1500];
    snprintf(request, sizeof(request),
             "GET /%s HTTP/1.0\r\nHost: %s\r\n\r\n", path[1] ? path + 1 : "", host);
    write(sock, request, strlen(request));

    struct termios orig_term;
    tcgetattr(STDIN_FILENO, &orig_term);
    set_raw_mode(&orig_term);

    char buf[BUF_SIZE];
    int lines = 0, pause = 0;

    fd_set fds;
    while (1) {
        FD_ZERO(&fds);
        if (!pause) FD_SET(sock, &fds);
        FD_SET(STDIN_FILENO, &fds);

        select(sock + 1, &fds, NULL, NULL, NULL);

        if (FD_ISSET(sock, &fds) && !pause) {
            int n = read(sock, buf, BUF_SIZE);
            if (n <= 0) break;

            for (int i = 0; i < n; ++i) {
                write(STDOUT_FILENO, &buf[i], 1);
                if (buf[i] == '\n' && ++lines >= LINES_PER_PAGE) {
                    pause = 1;
                    write(STDOUT_FILENO, "\nPress space to scroll down\n", 29);
                    break;
                }
            }
        }

        if (FD_ISSET(STDIN_FILENO, &fds)) {
            char c;
            if (read(STDIN_FILENO, &c, 1) > 0 && c == ' ') {
                lines = 0;
                pause = 0;
            }
        }
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &orig_term);
    close(sock);
    return 0;
}
