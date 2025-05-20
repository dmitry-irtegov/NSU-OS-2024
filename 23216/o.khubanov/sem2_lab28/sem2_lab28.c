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

#define INITIAL_BUF_SIZE 8192  // начальный размер динамического буфера
#define SCREEN_LINES 25        // количество строк на одном экране

// Структура динамического буфера для хранения приостановленных данных
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
    raw.c_cc[VMIN] = 0;   // минимум байт для read = 0
    raw.c_cc[VTIME] = 1;  // таймаут 0.1 с
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

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s URL\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Включаем raw-режим для немедленного считывания нажатий
    enable_raw_mode();

    char *host, *port, *path;
    parse_url(argv[1], &host, &port, &path);

    // Разрешение адреса через DNS
    struct addrinfo hints = {0}, *res;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(host, port, &hints, &res) != 0) {
        perror("getaddrinfo");
        exit(EXIT_FAILURE);
    }

    // Создание и подключение TCP-сокета
    int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock < 0) { perror("socket"); exit(EXIT_FAILURE); }
    if (connect(sock, res->ai_addr, res->ai_addrlen) < 0) { perror("connect"); exit(EXIT_FAILURE); }
    freeaddrinfo(res);

    // Формирование и отправка HTTP GET-запроса
    char req[1024];
    snprintf(req, sizeof(req), "GET %s HTTP/1.0\r\nHost: %s\r\n\r\n", path, host);
    free(host); free(port); free(path);
    write(sock, req, strlen(req));

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
        int maxfd = (sock > STDIN_FILENO ? sock : STDIN_FILENO) + 1;
        if (select(maxfd, &rd, NULL, NULL, NULL) < 0) {
            perror("select");
            break;
        }

        // Читаем из сетевого сокета
        if (!eof && FD_ISSET(sock, &rd)) {
            char tmp[4096];
            ssize_t r = read(sock, tmp, sizeof(tmp));
            if (r <= 0) {
                eof = 1;
                close(sock);
            } else {
                ssize_t i = 0;
                while (i < r) {
                    if (!paused) {
                        char c = tmp[i++];
                        write(STDOUT_FILENO, &c, 1);
                        if (c == '\n') {
                            lines++;
                            if (lines >= SCREEN_LINES) {
                                paused = 1;
                                lines = 0;
                                write(STDOUT_FILENO, "Press space to scroll down", 26);
                                fflush(stdout);
                                break;
                            }
                        }
                    } else {
                        buffer_append(&buf, tmp + i, (size_t)(r - i));
                        break;
                    }
                }
            }
        }

        // Обработка нажатия пробела
        if (paused && FD_ISSET(STDIN_FILENO, &rd)) {
            char c;
            if (read(STDIN_FILENO, &c, 1) > 0 && c == ' ') {
                paused = 0;
                write(STDOUT_FILENO, "\n", 1);
                int plines = 0;
                while (buf.pos < buf.len && plines < SCREEN_LINES) {
                    char d = buf.data[buf.pos++];
                    write(STDOUT_FILENO, &d, 1);
                    if (d == '\n') plines++;
                }
                if (buf.pos == buf.len) buf.len = buf.pos = 0;
                if (plines >= SCREEN_LINES && buf.pos < buf.len) {
                    paused = 1;
                    write(STDOUT_FILENO, "Press space to scroll down", 26);
                }
                fflush(stdout);
            }
        }

        // Если соединение закрыто и буфер пуст — выходим
        if (eof && buf.pos == buf.len) break;
    }

    buffer_free(&buf);
    return 0;
}

