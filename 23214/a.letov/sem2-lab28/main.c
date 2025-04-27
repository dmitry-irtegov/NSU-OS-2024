#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <termios.h>
#include <sys/select.h>

#define RINGBUF_SIZE 8000
char ringbuf[RINGBUF_SIZE];

void parse_url(const char *url, char *host, char *path) {
    if (strncmp(url, "http://", 7) == 0) {
        url += 7;
    }
    const char *slash = strchr(url, '/');
    if (!slash) {
        strcpy(host, url);
        strcpy(path, "/");
    } else {
        strncpy(host, url, slash - url);
        host[slash - url] = '\0';
        strcpy(path, slash);
    }
}

void echo_nonbuff_mode(int enable) {
    static struct termios old_t;
    struct termios new_t;
    if (enable) {
        tcgetattr(0, &old_t);
        new_t = old_t;
        new_t.c_lflag &= ~(ICANON | ECHO);
        new_t.c_cc[VMIN] = 1;
        tcsetattr(0, TCSANOW, &new_t);
    } else {
        tcsetattr(0, TCSANOW, &old_t);
    }
}

int ringbuf_put(int *start, int *end, char c) {
    if (((*end + 1) % RINGBUF_SIZE) != *start) {
        ringbuf[*end] = c;
        *end = (*end + 1) % RINGBUF_SIZE;
        return 1;
    } else {
        return 0;
    }
}

int ringbuf_full(int start, int end) {
    return ((end + 1) % RINGBUF_SIZE) == start;
}

int ringbuf_get(int *start, int *end, char *c) {
    if (*start == *end) {
        return 0;
    }
    *c = ringbuf[*start];
    *start = (*start + 1) % RINGBUF_SIZE;
    return 1;
}

int main(int argc, char *argv[]) {
    if (strncmp(argv[1], "http://", 7) != 0 || argc != 2) {
        fprintf(stderr, "BAD URL\n");
        exit(5);
    }
    char host[256], path[1024], request[2048], buffer[3500];
    struct addrinfo hints, *res;
    int sockfd;
    int new_line_cnt = 0;
    int paused = 0;
    int buf_start = 0;
    int buf_end = 0;
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
    char c;
    int data_done = 0;
    int header_parsed_check = 0;
    int stopped_index=0;
    int left_buffer=0;
    while (1) {
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        if (paused) FD_SET(0, &readfds);
        if (select(sockfd + 1, &readfds, NULL, NULL, NULL) < 0) {
            perror("select");
            exit(4);
        }
        if (FD_ISSET(sockfd, &readfds) && !left_buffer) {
            bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
            if (bytes_received <= 0) {
                data_done = 1;
            } else {
                if (!header_parsed_check) {
                    char *content = strstr(buffer, "\r\n\r\n");
                    buffer[bytes_received] = '\0';
                    if (content) {
                        content += 4;
                        int len = strlen(content);
                        for (int i = 0; i < len; ++i) {
                            if (!ringbuf_put(&buf_start, &buf_end, content[i])) {
                                left_buffer = 1;
                                stopped_index = i;
                                break;
                            }
                        }
                        if (!left_buffer) {
                            header_parsed_check = 1;
                        }
                    } 
                } else {
                    for (int i = 0; i < bytes_received; ++i) {
                        if (!ringbuf_put(&buf_start, &buf_end, buffer[i])) {
                            left_buffer = 1;
                            stopped_index = i;
                            break;
                        }
                    }
                }
            }
        }

        if (left_buffer) {
            while (stopped_index < bytes_received) {
                if (!ringbuf_put(&buf_start, &buf_end, buffer[stopped_index])) {
                    break;
                }
                stopped_index++;
            }
            if (stopped_index == bytes_received) {
                left_buffer = 0;
                stopped_index = 0;
            }
        }


        while (!paused && ringbuf_get(&buf_start, &buf_end, &c)) {
            putchar(c);
            if (c == '\n') {
                new_line_cnt++;
                if (new_line_cnt >= 25) {
                    printf("\nPress space to scroll down...");
                    fflush(stdout);
                    paused = 1;
                    break;
                }
            }
        }
        if (paused && FD_ISSET(0, &readfds)) {
            char input;
            if (read(0, &input, 1) > 0 && input == ' ') {
                new_line_cnt = 0;
                paused = 0;
            }
        }
        if (data_done && (buf_start == buf_end)) {
            printf("\nPRINTING IS DONE\n");
            break;
        }
    }
    close(sockfd);
    echo_nonbuff_mode(0);
    return 0;
}