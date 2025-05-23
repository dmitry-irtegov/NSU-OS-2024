#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <termios.h>
#include <sys/select.h>
#include <signal.h>

#define BUF_SIZE 4096
#define LINES_PER_PAGE 25

// Глобальная переменная для восстановления терминала при сигнале
struct termios orig_term;

// Обработчик сигнала
void handle_signal(int signum) {
    restore_terminal_mode(&orig_term);
    printf("\n[!] Caught signal %d (%s). Exiting gracefully...\n", signum, strsignal(signum));
    exit(1);
}

// Установка всех обработчиков сигналов
void setup_signal_handlers() {
    for (int i = 1; i <= 64; ++i) {
        if (i == SIGKILL || i == SIGSTOP || i == SIGCONT)
            continue; // Эти нельзя перехватить
        signal(i, handle_signal);
    }
}

// Установка "сырого" режима терминала
void set_terminal_mode(struct termios *orig_term) {
    struct termios raw = *orig_term;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

// Восстановление терминала
void restore_terminal_mode(struct termios *orig_term) {
    tcsetattr(STDIN_FILENO, TCSANOW, orig_term);
}

// Подключение по TCP к хосту
int connect_to_host(const char *host, const char *port) {
    struct addrinfo hints;
    struct addrinfo *res;
    int sock;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(host, port, &hints, &res) != 0) {
        perror("getaddrinfo");
        exit(1);
    }

    sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock < 0) {
        perror("socket");
        exit(1);
    }

    if (connect(sock, res->ai_addr, res->ai_addrlen) < 0) {
        perror("connect");
        close(sock);
        exit(1);
    }

    freeaddrinfo(res);
    return sock;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s http://host/path\n", argv[0]);
        return 1;
    }

    setup_signal_handlers();  // <-- Устанавливаем обработку сигналов

    char host[256], path[1024];
    path[0] = '/'; path[1] = '\0';
    if (sscanf(argv[1], "http://%255[^/]%1023s", host, path + 1) < 1) {
        fprintf(stderr, "Invalid URL format\n");
        return 1;
    }

    int sock = connect_to_host(host, "80");

    char request[1500];
    snprintf(request, sizeof(request),
             "GET /%s HTTP/1.0\r\nHost: %s\r\nConnection: close\r\n\r\n",
             path[1] ? path + 1 : "", host);
    write(sock, request, strlen(request));

    tcgetattr(STDIN_FILENO, &orig_term);  // Сохраняем текущие настройки терминала
    set_terminal_mode(&orig_term);        // Переводим в "сырой" режим

    char buf[BUF_SIZE];
    int lines = 0;
    int pause = 0;

    while (1) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(sock, &fds);
        FD_SET(STDIN_FILENO, &fds);

        int maxfd = (sock > STDIN_FILENO) ? sock : STDIN_FILENO;

        if (select(maxfd + 1, &fds, NULL, NULL, NULL) < 0) {
            perror("select");
            break;
        }

        if (FD_ISSET(STDIN_FILENO, &fds)) {
            char c;
            if (read(STDIN_FILENO, &c, 1) > 0 && c == ' ') {
                lines = 0;
                pause = 0;
            }
        }

        if (FD_ISSET(sock, &fds) && !pause) {
            int n = read(sock, buf, BUF_SIZE);
            if (n <= 0) break;

            for (int i = 0; i < n; ++i) {
                write(STDOUT_FILENO, &buf[i], 1);
                if (buf[i] == '\n') {
                    lines++;
                    if (lines >= LINES_PER_PAGE) {
                        pause = 1;
                        const char *msg = "\n-- Press space to scroll down --\n";
                        write(STDOUT_FILENO, msg, strlen(msg));
                        break;
                    }
                }
            }
        }
    }

    restore_terminal_mode(&orig_term);
    close(sock);
    return 0;
}
