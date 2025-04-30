#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <termios.h>
#include <sys/select.h>

#define DEFAULT_PORT "80"
#define SCREEN_LINES 25

void parser(const char *url, char *host, char *path, char *port) {
    const char *protocol = strstr(url, "://");
    const char *remaining = url;
    
    if (protocol) {
        remaining = protocol + 3;
    }

    const char *port_start = strchr(remaining, ':');
    const char *path_start = strchr(remaining, '/');

    if (port_start && (!path_start || port_start < path_start)) {
        strncpy(host, remaining, port_start - remaining);
        host[port_start - remaining] = '\0';
        
        const char *port_end = path_start ? path_start : strchr(port_start, '\0');
        strncpy(port, port_start + 1, port_end - (port_start + 1));
        port[port_end - (port_start + 1)] = '\0';
        
        remaining = port_end;
    } else {
        strcpy(port, DEFAULT_PORT);
    }

    if (path_start) {
        strcpy(path, path_start);
    } else {
        strcpy(path, "/");
        remaining = strchr(remaining, '\0');
    }

    if (!port_start || (path_start && port_start > path_start)) {
        strncpy(host, remaining, path_start ? path_start - remaining : strlen(remaining));
        host[path_start ? path_start - remaining : strlen(remaining)] = '\0';
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
    char port[6] = {0};
    char request[2048];
    char buffer[4096];
    
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <URL>\n", argv[0]);
        return 1;
    }

    parser(argv[1], host, path, port);

    struct addrinfo hints = {0};
    struct addrinfo *results;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int status;
    if ((status = getaddrinfo(host, port, &hints, &results)) != 0) {
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
    char prev = 0;
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
                        if (body[i] == '\n') {  // Изменено здесь
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