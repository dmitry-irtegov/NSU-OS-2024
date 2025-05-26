#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <termios.h>

#define BUFFER_SIZE 4096
#define LINES_PER_SCREEN 25
#define DEFAULT_HTTP_PORT "80"

static struct termios original_terminal;

void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_terminal);
}

void enable_raw_mode() {
    struct termios raw;
    tcgetattr(STDIN_FILENO, &original_terminal);
    atexit(disable_raw_mode);
    raw = original_terminal;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void parse_url(const char *url, char *host, char *port, char *path) {
    if (strncmp(url, "http://", 7) != 0) {
        fprintf(stderr, "Only http:// URLs are supported.\n");
        exit(EXIT_FAILURE);
    }

    const char *host_start = url + 7;
    const char *path_start = strchr(host_start, '/');
    if (!path_start) {
        strcpy(path, "/");
        path_start = host_start + strlen(host_start);
    } else {
        strcpy(path, path_start);
    }

    char host_port[256];
    strncpy(host_port, host_start, path_start - host_start);
    host_port[path_start - host_start] = '\0';

    char *colon = strchr(host_port, ':');
    if (colon) {
        *colon = '\0';
        strcpy(host, host_port);
        strcpy(port, colon + 1);
    } else {
        strcpy(host, host_port);
        strcpy(port, DEFAULT_HTTP_PORT);
    }
}

int connect_to_host(const char *host, const char *port) {
    struct addrinfo hints = {0}, *res, *rp;
    int sockfd;

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    getaddrinfo(host, port, &hints, &res);

    for (rp = res; rp != NULL; rp = rp->ai_next) {
        sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sockfd == -1) continue;
        if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) == 0) break;
        close(sockfd);
    }

    if (!rp) {
        fprintf(stderr, "Failed to connect to %s:%s\n", host, port);
        exit(1);
    }

    freeaddrinfo(res);
    return sockfd;
}

void send_request(int sockfd, const char *host, const char *path) {
    char request[1024];
    snprintf(request, sizeof(request),
             "GET %s HTTP/1.0\r\n"
             "Host: %s\r\n"
             "Connection: close\r\n"
             "\r\n",
             path, host);
    send(sockfd, request, strlen(request), 0);
}

void read_response(int sockfd) {
    char buffer[BUFFER_SIZE];
    int bytes_read;
    int line_count = 0;
    int in_body = 0;

    fd_set read_fds;
    int maxfd = sockfd > STDIN_FILENO ? sockfd : STDIN_FILENO;

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);
        FD_SET(STDIN_FILENO, &read_fds);

        if (select(maxfd + 1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            break;
        }

        if (FD_ISSET(sockfd, &read_fds)) {
            bytes_read = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
            if (bytes_read <= 0) break;
            buffer[bytes_read] = '\0';

            char *line = strtok(buffer, "\n");
            while (line) {
                if (!in_body) {
                    if (strcmp(line, "\r") == 0 || strcmp(line, "") == 0)
                        in_body = 1;
                } else {
                    printf("%s\n", line);
                    fflush(stdout);
                    if (++line_count == LINES_PER_SCREEN) {
                        printf("\nPress space to scroll down...\n");
                        fflush(stdout);
                        while (1) {
                            FD_ZERO(&read_fds);
                            FD_SET(STDIN_FILENO, &read_fds);
                            if (select(STDIN_FILENO + 1, &read_fds, NULL, NULL, NULL) > 0) {
                                char c;
                                read(STDIN_FILENO, &c, 1);
                                if (c == ' ') {
                                    line_count = 0;
                                    break;
                                }
                            }
                        }
                    }
                }
                line = strtok(NULL, "\n");
            }
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s URL\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    enable_raw_mode();
    setbuf(stdin, NULL);

    char host[256], port[10], path[1024];
    parse_url(argv[1], host, port, path);

    int sockfd = connect_to_host(host, port);
    send_request(sockfd, host, path);
    read_response(sockfd);
    close(sockfd);

    disable_raw_mode();

    exit(EXIT_SUCCESS);
}
