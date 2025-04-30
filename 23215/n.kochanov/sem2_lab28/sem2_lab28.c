#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <termios.h>
#include <sys/select.h>

#define SCREEN_LINES 25
#define DEFAULT_PORT 80

void parse_url(const char *url, char *host, char *path, int *port) {
    *port = DEFAULT_PORT;

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

void configure_terminal(int enable) {
    static struct termios original_settings;
    struct termios new_settings;

    if (enable) {
        tcgetattr(STDIN_FILENO, &original_settings);
        new_settings = original_settings;
        new_settings.c_lflag &= ~(ICANON | ECHO);
        new_settings.c_cc[VMIN] = 1;
        tcsetattr(STDIN_FILENO, TCSANOW, &new_settings);
    } else {
        tcsetattr(STDIN_FILENO, TCSANOW, &original_settings);
    }
}

int main(int argc, char *argv[]) {
    char host[256] = {0};
    char path[1024] = {0};
    int port;
    char request[2048];
    char buffer[4096];
    char port_str[6] = {0};
    
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <URL>\n", argv[0]);
        return 1;
    }

    parse_url(argv[1], host, path, &port);
    snprintf(port_str, sizeof(port_str), "%d", port);

    struct addrinfo hints = {0};
    struct addrinfo *results;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int status;
    if ((status = getaddrinfo(host, port_str, &hints, &results)) != 0) {
        fprintf(stderr, "Address resolution error: %s\n", gai_strerror(status));
        return 1;
    }

    int connection_fd = -1;
    struct addrinfo *addr;
    for (addr = results; addr != NULL; addr = addr->ai_next) {
        connection_fd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if (connection_fd == -1) continue;
        
        if (connect(connection_fd, addr->ai_addr, addr->ai_addrlen) == 0) {
            break;
        }
        close(connection_fd);
    }

    if (!addr) {
        fprintf(stderr, "Connection failed\n");
        freeaddrinfo(results);
        return 2;
    }
    freeaddrinfo(results);

    snprintf(request, sizeof(request), 
             "GET %s HTTP/1.0\r\nHost: %s\r\n\r\n", 
             path, host);
    send(connection_fd, request, strlen(request), 0);

    configure_terminal(1);

    int line_count = 0;
    int pause_mode = 0;
    int body_started = 0;
    fd_set descriptors;

    while (1) {
        FD_ZERO(&descriptors);
        FD_SET(connection_fd, &descriptors);
        if (pause_mode) FD_SET(STDIN_FILENO, &descriptors);

        if (select(connection_fd + 1, &descriptors, NULL, NULL, NULL) < 0) {
            perror("Selection error");
            break;
        }

        if (FD_ISSET(connection_fd, &descriptors) && !pause_mode) {
            int bytes = recv(connection_fd, buffer, sizeof(buffer) - 1, 0);
            if (bytes <= 0) break;
            buffer[bytes] = '\0';

            if (!body_started) {
                char *header_end = strstr(buffer, "\r\n\r\n");
                if (header_end) {
                    body_started = 1;
                    char *body = header_end + 4;
                    int body_len = bytes - (body - buffer);
                    for (int i = 0; i < body_len; i++) {
                        putchar(body[i]);
                        if (body[i] == '\n') {
                            if (++line_count >= SCREEN_LINES) {
                                printf("\n-- PAUSED (press space) --");
                                fflush(stdout);
                                pause_mode = 1;
                                line_count = 0;
                                break;
                            }
                        }
                        if (pause_mode) break;
                    }
                }
            } else {
                for (int i = 0; i < bytes; i++) {
                    putchar(buffer[i]);
                    if (buffer[i] == '\n') {
                        if (++line_count >= SCREEN_LINES) {
                            printf("\n-- PAUSED (press space) --");
                            fflush(stdout);
                            pause_mode = 1;
                            line_count = 0;
                            break;
                        }
                    }
                    if (pause_mode) break;
                }
            }
            fflush(stdout);
        }

        if (pause_mode && FD_ISSET(STDIN_FILENO, &descriptors)) {
            char input;
            read(STDIN_FILENO, &input, 1);
            if (input == ' ') {
                pause_mode = 0;
                printf("\n");
                fflush(stdout);
            }
        }
    }

    close(connection_fd);
    configure_terminal(0);
    return 0;
}