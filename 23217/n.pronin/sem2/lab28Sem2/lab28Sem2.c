#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/select.h>
#include <ctype.h>
#include <iconv.h>

#define BUFFER_SIZE 4096
#define LINES_PER_PAGE 25

void error(const char *msg) {
    perror(msg);
    exit(1);
}

// Функция для извлечения хоста и пути из URL
void parse_url(const char *url, char *host, char *path) {
    if (strncmp(url, "http://", 7) == 0)
        url += 7;

    const char *slash = strchr(url, '/');
    if (slash) {
        strncpy(host, url, slash - url);
        host[slash - url] = '\0';
        strcpy(path, slash);
    } else {
        strcpy(host, url);
        strcpy(path, "/";
    }
}

int create_connection(const char *host) {
    struct hostent *server = gethostbyname(host);
    if (!server) error("No such host");

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("Error opening socket");

    struct sockaddr_in serv_addr = {0};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(80);
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("Connection failed");

    return sockfd;
}

void send_request(int sockfd, const char *host, const char *path) {
    char request[1024];
    snprintf(request, sizeof(request),
             "GET %s HTTP/1.0\r\nHost: %s\r\nConnection: close\r\n\r\n",
             path, host);
    write(sockfd, request, strlen(request));
}

char* extract_encoding(const char *response) {
    const char *content_type = strstr(response, "Content-Type:");
    if (content_type) {
        char *encoding = strstr(content_type, "charset=");
        if (encoding) {
            encoding += 8; // Пропустить "charset="
            char *end = strchr(encoding, ';');
            if (end) *end = '\0'; // Завершить строку
            return strdup(encoding); // Вернуть кодировку
        }
    }
    return NULL; // Если кодировка не найдена
}

void convert_encoding(char *input, size_t input_len, char *output, size_t output_len, const char *from_encoding) {
    iconv_t cd = iconv_open("UTF-8", from_encoding);
    if (cd == (iconv_t)(-1)) {
        error("iconv_open failed");
    }

    const char *in_buf_const = input;
    char *out_buf = output;
    size_t in_bytes_left = input_len;
    size_t out_bytes_left = output_len;

    size_t res = iconv(cd, &in_buf_const, &in_bytes_left, &out_buf, &out_bytes_left);
    if (res == (size_t)(-1)) {
        perror("iconv");
    }

    iconv_close(cd);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <URL>\n", argv[0]);
        exit(1);
    }

    char host[256], path[1024];
    parse_url(argv[1], host, path);

    int sockfd = create_connection(host);
    send_request(sockfd, host, path);

    fd_set read_fds;
    char buffer[BUFFER_SIZE];
    int lines_printed = 0;
    int paused = 0;

    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
    char response[BUFFER_SIZE];
    int total_bytes = 0;

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        int maxfd = sockfd > STDIN_FILENO ? sockfd : STDIN_FILENO;

        int activity = select(maxfd + 1, &read_fds, NULL, NULL, NULL);
        if (activity < 0) error("select");

        // Обработка данных от сервера
        if (FD_ISSET(sockfd, &read_fds) && !paused) {
            int n = read(sockfd, buffer, sizeof(buffer) - 1);
            if (n <= 0) break; // соединение закрыто
            buffer[n] = '\0';

            // Сохраняем ответ для анализа кодировки
            strncat(response, buffer, sizeof(response) - strlen(response) - 1);
            total_bytes += n;

            // Проверяем, получили ли мы полный заголовок
            if (strstr(response, "\r\n\r\n")) {
                char *encoding = extract_encoding(response);
                if (encoding) {
                    // Преобразуем кодировку
                    char output_buffer[BUFFER_SIZE * 2];
                    convert_encoding(buffer, n, output_buffer, sizeof(output_buffer), encoding);

                    // Вывод построчно
                    char *line = strtok(output_buffer, "\n");
                    while (line) {
                        printf("%s\n", line);
                        fflush(stdout);
                        lines_printed++;
                        if (lines_printed >= LINES_PER_PAGE) {
                            printf("Press space to scroll down...\n");
                            fflush(stdout);
                            paused = 1;
                            break;
                        }
                        line = strtok(NULL, "\n");
                    }
                    free(encoding); // Освобождаем память
                }
            }
        }

        // Обработка ввода пользователя
        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            char ch;
            if (read(STDIN_FILENO, &ch, 1) > 0) {
                if (paused && ch == ' ') {
                    lines_printed = 0;
                    paused = 0;
                }
            }
        }
    }

    close(sockfd);
    return 0;
}
