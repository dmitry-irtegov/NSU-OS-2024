#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/select.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>


#define INITIAL_BUF_SIZE 8192  // начальный размер динамического буфера
#define SCREEN_LINES 25        // количество строк на одном экране


static struct termios orig_termios;  // для сохранения исходных настроек терминала

// --- Обёртки системных вызовов ---
ssize_t xread(int fd, void *buf, size_t count) {
    ssize_t r;
    do { r = read(fd, buf, count); } while (r < 0 && errno == EINTR);
    return r;
}

ssize_t xwrite(int fd, const void *buf, size_t count) {
    size_t done = 0;
    while (done < count) {
        ssize_t w = write(fd, (const char*)buf + done, count - done);
        if (w < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        done += w;
    }
    return done;
}

int xselect(int nfds, fd_set *readfds) {
    int r;
    do { r = select(nfds, readfds, NULL, NULL, NULL); } while (r < 0 && errno == EINTR);
    return r;
}

typedef struct {
    char *data;
    size_t size;
    size_t len;
    size_t pos;
} Buffer;

// Сохраняем исходные настройки терминала для восстановления
typedef struct termios Termios;
static Termios orig_termios;

// Отключаем режим raw при выходе
void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

// Переводим терминал в raw-режим (неканонический, без эха)
void enable_raw_mode() {
    Termios raw;
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disable_raw_mode);
    raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 1;   
    raw.c_cc[VTIME] = 0;  
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

// Инициализация буфера
void buffer_init(Buffer *buf) {
    buf->size = INITIAL_BUF_SIZE;
    buf->data = malloc(buf->size);
    buf->len = buf->pos = 0;
}

// Добавление данных в буфер
void buffer_append(Buffer *buf, const char *data, size_t n) {
    if (buf->len + n > buf->size) {
        while (buf->len + n > buf->size) buf->size *= 2;
        buf->data = realloc(buf->data, buf->size);
    }
    memcpy(buf->data + buf->len, data, n);
    buf->len += n;
}

// Очистка буфера
void buffer_free(Buffer *buf) {
    free(buf->data);
}

// Разбор URL http://host[:port]/path
void parse_url(const char *url, char **host, char **port, char **path) {
    const char *p = (strncmp(url, "http://", 7) == 0) ? url + 7 : url;
    const char *slash = strchr(p, '/');
    if (slash) {
        *host = strndup(p, slash - p);
        *path = strdup(slash);
    } else {
        *host = strdup(p);
        *path = strdup("/");
    }
    char *colon = strchr(*host, ':');
    if (colon) {
        *colon = '\0';
        *port = strdup(colon + 1);
    } else {
        *port = strdup("80");
    }
}

// --- Сетевые операции ---
int connect_to_host(const char *host, const char *port) {
    struct addrinfo hints = {0}, *res;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(host, port, &hints, &res)) {
        perror("getaddrinfo"); exit(EXIT_FAILURE);
    }
    int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock < 0) { perror("socket"); exit(EXIT_FAILURE); }
    if (connect(sock, res->ai_addr, res->ai_addrlen) < 0) { perror("connect"); exit(EXIT_FAILURE); }
    freeaddrinfo(res);
    return sock;
}

void send_http_request(int sock, const char *host, const char *path) {
    // Формирование и отправка HTTP GET-запроса
    char req[1024];
    snprintf(req, sizeof(req),
             "GET %s HTTP/1.0\r\nHost: %s\r\n\r\n",
             path, host);
    if (xwrite(sock, req, strlen(req)) < 0) { perror("write"); exit(EXIT_FAILURE); }
}

// --- Цикл пейджера ---
void pager_loop(int sock) {
    Buffer buf;
    buffer_init(&buf);

    int lines = 0;
    int paused = 0;
    int eof = 0;

    while (1) {
        fd_set rd;
        FD_ZERO(&rd);
        if (!eof) FD_SET(sock, &rd);
        FD_SET(STDIN_FILENO, &rd);
        int maxfd = sock > STDIN_FILENO ? sock : STDIN_FILENO;

        xselect(maxfd + 1, &rd);

        // Чтение из сокета
        if (!eof && FD_ISSET(sock, &rd)) {
            char tmp[4096];
            ssize_t r = xread(sock, tmp, sizeof(tmp));
            if (r <= 0) {
                eof = 1;
                close(sock);
            } else {
                for (ssize_t i = 0; i < r; i++) {
                    if (!paused) {
                        xwrite(STDOUT_FILENO, &tmp[i], 1);
                        if (tmp[i] == '\n' && ++lines >= SCREEN_LINES) {
                            paused = 1;
                            lines = 0;
                            const char *prompt1 = "Нажмите пробел ";
			    xwrite(STDOUT_FILENO, prompt1, strlen(prompt1));
			    fflush(stdout);
                            break;
                        }
                    } else {
                        buffer_append(&buf, tmp + i, 1);
                    }
                }
            }
        }

        // Обработка нажатия пробела
        if (paused && FD_ISSET(STDIN_FILENO, &rd)) {
            char c;
            if (xread(STDIN_FILENO, &c, 1) > 0 && c == ' ') {
                paused = 0;
                xwrite(STDOUT_FILENO, "\n", 1);
                int plines = 0;
                while (buf.pos < buf.len && plines < SCREEN_LINES) {
                    char d = buf.data[buf.pos++];
                    xwrite(STDOUT_FILENO, &d, 1);
                    if (d == '\n') plines++;
                }
                if (buf.pos == buf.len) buf.len = buf.pos = 0;
                if (plines >= SCREEN_LINES && buf.pos < buf.len) {
                    paused = 1;
		    const char *prompt2 = "Нажмите пробел для продолжения";
    		    xwrite(STDOUT_FILENO, prompt2, strlen(prompt2));
                }
                fflush(stdout);
            }
        }

        // Выход из цикла после EOF
        if (eof && buf.pos == buf.len) break;
    }

    buffer_free(&buf);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Использование: %s URL\n", argv[0]);
        return EXIT_FAILURE;
    }

    enable_raw_mode();  // переводим терминал в raw-режим

    char *host, *port, *path;
    parse_url(argv[1], &host, &port, &path);

    int sock = connect_to_host(host, port);
    send_http_request(sock, host, path);

    free(host);
    free(port);
    free(path);

    pager_loop(sock);

    return EXIT_SUCCESS;
}
