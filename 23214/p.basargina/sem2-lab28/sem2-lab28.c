#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <termios.h>

#define BUFSZ 4096
#define MAX_USER_BUF (7 * 1024 * 1024)
#define MAX_SCREEN_LINES 25
struct termios orig_term, raw_term;

void enable_raw_mode() {
    if (tcgetattr(STDIN_FILENO, &orig_term) == -1) {
        perror("tcgetattr failed");
        exit(1);
    }

    raw_term = orig_term;
    raw_term.c_lflag &= ~(ICANON | ECHO);
    raw_term.c_cc[VMIN]  = 1;

    if (tcsetattr(STDIN_FILENO, TCSANOW, &raw_term) == -1) {
        perror("tcsetattr failed");
        exit(1);
    }
}

void restore_termios() {
    if (tcsetattr(STDIN_FILENO, TCSANOW, &orig_term) == -1) {
        perror("tcsetattr failed");
        exit(1);
    }
}

void parse_url(char *url, char **host, char **port, char **path) {
    char *scheme = "http://";
    size_t scheme_len = strlen(scheme);

    if (strncmp(url, scheme, scheme_len) != 0) {
        fprintf(stderr, "Only http:// URLs are supported\n");
        exit(1);
    }

    char *p = url + scheme_len;
    char *slash = strchr(p, '/');
    char *colon = strchr(p, ':');

    if (slash == NULL) { // Path is missing
        slash = p + strlen(p);
    }
    if (colon && colon < slash) {
        *host = strndup(p, colon - p);
        *port = strndup(colon + 1, slash - colon - 1);
    } else {
        *host = strndup(p, slash - p);
        *port = strdup("80");
    }

    *path = (*slash) ? strdup(slash) : strdup("/");
    printf("path: %s\n", *path);
    printf("host: %s, port: %s\n", *host, *port);
}

// dynamic buffer for pending data
typedef struct {
    char   *data;
    size_t  len;
    size_t  capacity;
} DynamicBuffer;

void buffer_init(DynamicBuffer *buf) {
    buf->data = NULL;
    buf->len = 0;
    buf->capacity = 0;
}

void buffer_free(DynamicBuffer *buf) {
    free(buf->data);
}

void buffer_append(DynamicBuffer *buf, char *src, size_t n) {
    if (buf->len + n >= buf->capacity) {
        size_t new_capacity = buf->capacity ? buf->capacity * 2 : 8192;

        while (new_capacity < buf->len + n) {
            new_capacity *= 2;
        }

        new_capacity = new_capacity > MAX_USER_BUF ? MAX_USER_BUF : new_capacity;

        char *new_data = realloc(buf->data, new_capacity);
        if (!new_data) {
            perror("realloc failed");
            exit(1);
        }
        buf->data = new_data;
        buf->capacity = new_capacity;
    }

    memcpy(buf->data + buf->len, src, n);
    buf->len += n;
}

int buffer_display_lines(DynamicBuffer *buf, int lines_left) {
    int printed = 0;
    size_t i;

    for (i = 0; i < buf->len && printed < lines_left; i++) {
        putchar(buf->data[i]);
        if (buf->data[i] == '\n') {
            ++printed;
        }
    }

    if (i > 0) {
        memmove(buf->data, buf->data + i, buf->len - i);
        buf->len -= i;
    }

    return printed;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s http://host[:port]/path\n", argv[0]);
        exit(1);
    }
    tcflush(STDIN_FILENO, TCIFLUSH);

    // parse url
    char *host, *port, *path = NULL;
    parse_url(argv[1], &host, &port, &path);

    // connect
    struct addrinfo hints, *res = NULL;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int status = getaddrinfo(host, port, &hints, &res);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo failed: %s\n", gai_strerror(status));
        exit(1);
    }

    int sockfd = -1;
    for (struct addrinfo *res_i = res; res_i; res_i = res_i->ai_next) {
        sockfd = socket(res_i->ai_family, res_i->ai_socktype, res_i->ai_protocol);
        if (sockfd == -1) {
            continue;
        }
        if (connect(sockfd, res_i->ai_addr, res_i->ai_addrlen) == 0) {
            break;
        }
        close(sockfd); 
        sockfd = -1;
    }
    freeaddrinfo(res);
    if (sockfd == -1) {
        perror("connect failed");
        exit(1);
    }

    // send HTTP GET
    char request[1024];
    int n = snprintf(request, sizeof(request),
                     "GET %s HTTP/1.1\r\n"
                     "Host: %s\r\n"
                     "Connection: close\r\n\r\n",
                     path, host);

    if (write(sockfd, request, n) != n) {
        perror("write failed");
        close(sockfd);
        exit(1);
    }

    // prepare terminal
    enable_raw_mode();

    // select loop
    DynamicBuffer buf;
    buffer_init(&buf);

    int lines_on_screen = 0;
    int is_paused = 0;
    int headers_done = 0;
    int sock_eof = 0;

    while (!sock_eof || buf.len) {
        fd_set read_fds; 
        FD_ZERO(&read_fds);

        size_t free_space = MAX_USER_BUF - buf.len;
        int want_socket = !sock_eof && free_space > 0;

        if (!sock_eof && want_socket) {
            FD_SET(sockfd, &read_fds);
        }
        if (is_paused) {
            FD_SET(STDIN_FILENO, &read_fds);
        }

        int nfds = (sockfd > STDIN_FILENO ? sockfd : STDIN_FILENO) + 1;

        if (select(nfds, &read_fds, NULL, NULL, NULL) == -1) {
            restore_termios();
            perror("select failed");
            close(sockfd);
            exit(1);
        }

        // read from socket
        if (FD_ISSET(sockfd, &read_fds)) {
            char input[BUFSZ];
            size_t chunk = free_space < BUFSZ ? free_space : BUFSZ;
            ssize_t bytes_read  = read(sockfd, input, chunk);

            if (bytes_read <= 0) {
                if (bytes_read < 0) {
                    perror("read failed");
                }
                sock_eof = 1;
                close(sockfd);
            } else {
                char *ptr = input;
                size_t ptr_len = (size_t)bytes_read;

                // skip headers
                if (!headers_done) {
                    char *hdr_end = NULL;
                    for (size_t i = 0; i + 3 < ptr_len; i++) {
                        if (input[i]== '\r' && input[i+1]=='\n' &&
                            input[i+2]=='\r' && input[i+3]=='\n') {
                            hdr_end = input + i + 4;
                            break;
                        }
                    }
                    if (hdr_end) {
                        headers_done = 1;
                        size_t hdr_skip = hdr_end - input;
                        ptr += hdr_skip;
                        ptr_len -= hdr_skip;
                    } else {
                        // still in headers
                        ptr_len = 0;
                    }
                }
                if (ptr_len > 0) {
                    buffer_append(&buf, ptr, ptr_len);
                }
            }
        }

        // keypresses
        if (FD_ISSET(STDIN_FILENO, &read_fds) && is_paused) {
            char c;
            if (read(STDIN_FILENO, &c, 1) == 1 && c == ' ') {
                is_paused = 0;
                lines_on_screen = 0;
            }
        }

        // print lines if not paused
        if (!is_paused && buf.len > 0) {
            int printed = buffer_display_lines(&buf, MAX_SCREEN_LINES - lines_on_screen);
            lines_on_screen += printed;

            if (lines_on_screen == MAX_SCREEN_LINES) {
                printf("\nPress space to scroll down\n");
                tcflush(STDIN_FILENO, TCIFLUSH);
                is_paused = 1;
            }
        }
    }

    free(host);
    free(port);
    free(path);
    buffer_free(&buf);
    restore_termios();
    close(sockfd);
    exit(0);
}