#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>
#include <stdbool.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <signal.h>
#include <semaphore.h>

#define BUFFER_SIZE 1000
#define BUFFER_PART_SIZE 20

typedef struct {
    char *buffer;
    int capacity;
    int head;
    int tail;
    int size;
} ring_buffer_t;

int sock;
ring_buffer_t *rb;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
bool is_reading_done = false;
int turn = 0;

struct termios orig_termios;
int orig_fl;

void make_stdin_nonblocking(void) {
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl get");
        exit(EXIT_FAILURE);
    }
    if (fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl set");
        exit(EXIT_FAILURE);
    }
}

void enable_raw_mode(void) {
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN]  = 1;
    raw.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }
    orig_fl = fcntl(STDIN_FILENO, F_GETFL);
    if (orig_fl == -1) {
        perror("fcntl F_GETFL");
        exit(EXIT_FAILURE);
    }
    if (fcntl(STDIN_FILENO, F_SETFL, orig_fl | O_NONBLOCK) == -1) {
        perror("fcntl F_SETFL");
        exit(EXIT_FAILURE);
    }
}

void disable_raw_mode(void) {
    tcflush(STDIN_FILENO, TCIFLUSH);
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }
    if (fcntl(STDIN_FILENO, F_SETFL, orig_fl) == -1) {
        perror("fcntl F_SETFL");
        exit(EXIT_FAILURE);
    }
}

void on_exit_signal(int sig) {
    disable_raw_mode();
    exit(EXIT_FAILURE);
}

ring_buffer_t *rb_create(int capacity) {
    ring_buffer_t *rb = malloc(sizeof(ring_buffer_t));
    if (rb == NULL) {
        return NULL;
    }
    rb->buffer = malloc(capacity);
    if (rb->buffer == NULL) {
        free(rb);
        return NULL;
    }
    rb->capacity = capacity;
    rb->head = 0;
    rb->tail = 0;
    rb->size = 0;
    return rb;
}

void rb_destroy(ring_buffer_t *rb) {
    if (rb == NULL) {
        return;
    }
    free(rb->buffer);
    free(rb);
}

int rb_read_from_sock() {
    int to_read = (rb->head >= rb->tail ? rb->capacity : rb->tail) - rb->head;
    if (to_read > BUFFER_PART_SIZE) {
        to_read = BUFFER_PART_SIZE;
    }
    pthread_mutex_unlock(&mutex);
    int n = read(sock, rb->buffer + rb->head, to_read);
    pthread_mutex_lock(&mutex);
    if (n > 0) {
        rb->head = (rb->head + n) % rb->capacity;
        rb->size += n;
    }
    pthread_cond_signal(&cond);
    return n;
}

int rb_print_lines(int max_lines) {
    int lines_count = 0, to_write;
    int max_to_write = rb->tail < rb->head ? rb->size : rb->capacity - rb->tail;
    for (to_write = 0; to_write < max_to_write && lines_count < max_lines; to_write++) {
        if (rb->buffer[rb->tail + to_write] == '\n') {
            lines_count += 1;
        }
    }
    pthread_mutex_unlock(&mutex);
    int n = write(STDOUT_FILENO, rb->buffer + rb->tail, to_write);
    pthread_mutex_lock(&mutex);
    if (n < 0) {
        return -1;
    }
    int real_lines_count = 0, i;
    for (i = 0; i < n; ++i) {
        if (rb->buffer[rb->tail + i] == '\n') {
            real_lines_count += 1;
        }
    }
    rb->tail = (rb->tail + n) % rb->capacity;
    rb->size -= n;
    pthread_cond_signal(&cond);
    return n > 0 ? real_lines_count : -1;
}

char *substr(char *str, int start, int end) {
    char *s;
    if ((s = malloc(sizeof(char) * (end - start + 1))) == NULL) {
        perror("malloc failed");
        return NULL;
    }
    memcpy(s, str + start, end - start);
    s[end - start] = '\0';
    return s;
}

int parse_url(char *url,
              char **protocol,
              char **user,
              char **port,
              char **hostname,
              char **uri)
{
    char *p;
    int i, len = strlen(url);

    *protocol = *user = *port = *hostname = *uri = NULL;

    if ((p = strstr(url, "://")) == NULL) {
        return -1;
    }
    if ((*protocol = substr(url, 0, p - url)) == NULL) {
        return -1;
    }
    p += 3;
    char *at = strstr(p, "@");
    char *uri_start = strstr(p, "/");
    if (uri_start == NULL) {
        uri_start = strstr(p, "?");
    }
    char *colon = strstr(p, ":");
    
    if (at && (!uri_start || at < uri_start)) {
        colon = strstr(at, ":");
        if ((*user = substr(p, 0, at - p)) == NULL) {
            free(*protocol);
            return -1;
        }
        p = at + 1;
    }
    if (colon && (uri_start && colon > uri_start)) {
        colon = NULL;
    }
    
    int hostport_end = uri_start ? (uri_start - p) : strlen(p);
    int host_end = colon ? (colon - p) : hostport_end;
    
    if (host_end == 0 || (*hostname = substr(p, 0, host_end)) == NULL) {
        free(*protocol);
        free(*user);
        return -1;
    }
    
    if (colon) {
        int is_port_valid = 1;
        int port_start = (colon - p) + 1;
        int port_len = hostport_end - port_start;
        if (port_len == 0) {
            is_port_valid = 0;
        }
        for (i = 0; i < port_len; i++) {
            if ('9' < p[port_start + i] || p[port_start + i] < '0') {
                is_port_valid = 0;
            }
        }
        if (!is_port_valid || (*port = substr(p, port_start, port_start + port_len)) == NULL) {
            free(*protocol);
            free(*user);
            free(*hostname);
            return -1;
        }
    } else {
        if ((*port = substr("80", 0, 2)) == NULL) {
            free(*protocol);
            free(*user);
            free(*hostname);
            return -1;
        }
    }
    
    if ((*uri = substr(uri_start ? uri_start : "/", 0, uri_start ? strlen(uri_start) : 1)) == NULL) {
        free(*protocol);
        free(*user);
        free(*hostname);
        free(*port);
        return -1;
    }
    
    return 0;
}

int open_ipv4_socket(char *hostname, char *port) {
    int sock = -1;
    struct addrinfo hints = { 0 }, *res, *rp;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    if (getaddrinfo(hostname, port, &hints, &res) != 0) {
        return -1;
    }
    for (rp = res; rp; rp = rp->ai_next) {
        sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sock < 0) {
            continue;
        }
        if (connect(sock, rp->ai_addr, rp->ai_addrlen) < 0) {
            close(sock);
            sock = -1;
            continue;
        }
        break;
    }
    freeaddrinfo(res);
    return sock;
}

int build_http_get_request(char *out_buf, int buf_size, const char *hostname, const char *uri) {
    int len = snprintf(out_buf, buf_size,
                       "GET %s HTTP/1.1\r\n"
                       "Host: %s\r\n"
                       "Connection: close\r\n"
                       "\r\n",
                       uri, hostname);
    if (len < 0 || len >= buf_size) {
        fprintf(stderr, "Error building request\n");
        return -1;
    }
    return len;
}

int send_all(int sock, char *buf, int len) {
    int total = 0;
    while (total < len) {
        int sent = send(sock, buf + total, len - total, 0);
        if (sent < 0) {
            perror("send");
            return -1;
        }
        total += sent;
    }
    return 0;
}


int read_response_header(int sock, char *buffer, int buf_size) {
    int received = 0;
    while (received + 1 < buf_size) {
        int n = recv(sock, buffer + received, 1, 0);
        if (n <= 0) {
            perror("recv");
            return -1;
        }
        received += n;
        buffer[received] = '\0';
        if (strstr(buffer, "\r\n\r\n") != NULL) {
            return received;
        }
    }
    printf("%s\n", buffer);
    return -1;
}

int send_http_get_and_read_header(int sock, char *hostname, char *uri) {
    char req[1000], resp[1000];
    int req_len = build_http_get_request(req, sizeof(req), hostname, uri);
    if (req_len == -1 || send_all(sock, req, req_len) < 0) {
        return -1;
    }
    int header_len = read_response_header(sock, resp, 1000);
    return header_len == -1 ? -1 : header_len;
}

void *reader_thread(void *arg) {
    int n;
    pthread_mutex_lock(&mutex);
    while (!is_reading_done) {
        while (rb->capacity == rb->size || turn != 0) {
            turn = 1;
            pthread_cond_signal(&cond);
            pthread_cond_wait(&cond, &mutex);
        }
        if ((n = rb_read_from_sock()) < 0) {
            perror("rb_read_from_sock failed");
            exit(EXIT_FAILURE);
        } else if (n == 0) {
            is_reading_done = true;
        }
        turn = 1;
        pthread_cond_signal(&cond);
    }
    pthread_mutex_unlock(&mutex);
    return 0;
}

void *writer_thread(void *arg) {
    int current_lines = 0;
    pthread_mutex_lock(&mutex);
    while (!is_reading_done || rb->size != 0) {
        if (rb->size == 0) {
            turn = 0;
            pthread_cond_signal(&cond);
        }
        while (rb->size == 0) {
            if (is_reading_done) {     
                pthread_mutex_unlock(&mutex);
                return 0;
            }
            pthread_cond_wait(&cond, &mutex);
        }
        if (current_lines == 25) {
            while (turn != 1) {
                pthread_cond_wait(&cond, &mutex);
            }
            char ch;
            int rd = read(STDIN_FILENO, &ch, 1);
            if (rd > 0 && ch == ' ') {
                current_lines = 0;
            }
            if (!is_reading_done) {
                turn = 0;
                pthread_cond_signal(&cond);
            }
            continue;
        }
        int n = rb_print_lines(25 - current_lines);
        if (n >= 0) {
            current_lines += n;
            if (current_lines == 25) {
                printf("Press space to scroll down\n");
            }
        } else {
            perror("rb_print_lines failed");
            exit(EXIT_FAILURE);
        }
    }
    pthread_mutex_unlock(&mutex);
    return 0;
}

int main(int argc, char *argv[]) {
    int fd;
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <url>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *protocol = NULL;
    char *user = NULL;
    char *port = NULL;
    char *hostname = NULL;
    char *uri = NULL;
    
    if (parse_url(argv[1], &protocol, &user, &port, &hostname, &uri) != 0) {
        fprintf(stderr, "parse_url failed\n");
        return EXIT_FAILURE;
    }

    printf("Protocol: %s\n", protocol);
    if (user) {
        printf("User: %s\n", user);
    }
    printf("Hostname: %s\n", hostname);
    printf("Port: %s\n", port);

    if (strcasecmp(protocol, "http") != 0) {
        fprintf(stderr, "Only 'http' protocol is supported\n");
        return EXIT_FAILURE;
    }
    
    if (user != NULL) {
        fprintf(stderr, "HTTP authentication is not supported\n");
        return EXIT_FAILURE;
    }
    
    sock = open_ipv4_socket(hostname, port);
    if (sock < 0) {
        fprintf(stderr, "Cannot connect to socket\n");
        return EXIT_FAILURE;
    }
    
    if (send_http_get_and_read_header(sock, hostname, uri) == -1) {
        fprintf(stderr, "Failed to send GET request or read header\n");
        return EXIT_FAILURE;
    }
    
    printf("URI: %s\nGET REQUEST BODY:\n", uri);
    
    rb = rb_create(BUFFER_SIZE);
    
    if (atexit(disable_raw_mode) != 0) {
        perror("atexit");
        exit(EXIT_FAILURE);
    }

    signal(SIGINT,  on_exit_signal);
    signal(SIGTERM, on_exit_signal);
    
    enable_raw_mode();
    make_stdin_nonblocking();
    
    int code;
    pthread_t reader, writer;
    if ((code = pthread_create(&reader, NULL, reader_thread, NULL)) != 0) {
        fprintf(stderr, "Failed to create thread: %d.\n", code);
        return EXIT_FAILURE;
    }
    if ((code = pthread_create(&writer, NULL, writer_thread, NULL)) != 0) {
        fprintf(stderr, "Failed to create thread: %d.\n", code);
        return EXIT_FAILURE;
    }

    if ((code = pthread_join(reader, NULL)) != 0) {
        fprintf(stderr, "Failed to join thread: %d.\n", code);
        return EXIT_FAILURE;
    }
    
    if ((code = pthread_join(writer, NULL)) != 0) {
        fprintf(stderr, "Failed to join thread: %d.\n", code);
        return EXIT_FAILURE;
    }
    
    close(sock);
    free(protocol);
    free(user);
    free(port);
    free(hostname);
    free(uri);
    rb_destroy(rb);
    disable_raw_mode();
    return EXIT_SUCCESS;
}

