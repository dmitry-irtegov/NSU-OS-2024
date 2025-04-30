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
#define BUFFER_SIZE 65536

typedef struct {
    char *data;
    size_t size;
    size_t capacity;
    size_t display_pos;
} ContentBuffer;

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

void buffer_append(ContentBuffer *buf, const char *data, size_t len) {
    while (buf->size + len + 1 > buf->capacity) {
        buf->capacity = buf->capacity ? buf->capacity * 2 : 4096;
        buf->data = realloc(buf->data, buf->capacity);
    }
    memcpy(buf->data + buf->size, data, len);
    buf->size += len;
    buf->data[buf->size] = '\0';
}

void buffer_init(ContentBuffer *buf) {
    buf->data = NULL;
    buf->size = 0;
    buf->capacity = 0;
    buf->display_pos = 0;
}

void buffer_free(ContentBuffer *buf) {
    free(buf->data);
    buf->data = NULL;
}

int main(int argc, char *argv[]) {
    char host[256] = {0};
    char path[1024] = {0};
    int port;
    char request[2048];
    char port_str[6] = {0};
    ContentBuffer content = {0};
    
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
    buffer_init(&content);

    int line_count = 0;
    int pause_mode = 0;
    int header_processed = 0;
    fd_set descriptors;

int connection_closed = 0;

while (1) {
    FD_ZERO(&descriptors);
    if (!connection_closed) FD_SET(connection_fd, &descriptors);
    FD_SET(STDIN_FILENO, &descriptors);

    if (select(connection_fd + 1, &descriptors, NULL, NULL, NULL) < 0) {
        perror("Selection error");
        break;
    }

    if (!connection_closed && FD_ISSET(connection_fd, &descriptors)) {
        char buffer[4096];
        int bytes = recv(connection_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) {
            connection_closed = 1;
            shutdown(connection_fd, SHUT_RD);
        } else {
            buffer[bytes] = '\0';
            buffer_append(&content, buffer, bytes);
        }
    }

    if (!header_processed) {
        char *header_end = strstr(content.data, "\r\n\r\n");
        if (header_end) {
            content.display_pos = header_end - content.data + 4;
            header_processed = 1;
        }
    }

    if (header_processed && !pause_mode) {
        size_t start = content.display_pos;
        size_t printed = 0;
        int lines_printed = 0;

        while (start + printed < content.size) {
            if (content.data[start + printed] == '\n') {
                lines_printed++;
                if (lines_printed >= SCREEN_LINES) {
                    printed++;
                    break;
                }
            }
            printed++;
        }

        if (printed > 0) {
            write(STDOUT_FILENO, content.data + start, printed);
            content.display_pos += printed;

            pause_mode = 1;
            printf("\n-- PAUSED (press space) --");
            fflush(stdout);
        } else if (connection_closed) {
            break;
        }
    }

    if (FD_ISSET(STDIN_FILENO, &descriptors) && pause_mode) {
        char input;
        read(STDIN_FILENO, &input, 1);
        if (input == ' ') {
            printf("\n");
            fflush(stdout);
            pause_mode = 0;
        }
    }
}

    close(connection_fd);
    configure_terminal(0);
    buffer_free(&content);
    return 0;
}