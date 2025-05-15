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
        strcpy(path, "/");
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

size_t convert_encoding(char *input, size_t input_len, char *output, size_t output_len) {
    iconv_t cd = iconv_open("UTF-8", "WINDOWS-1251");
    if (cd == (iconv_t)(-1)) {
        perror("iconv_open");
        return 0;
    }

    char *in_buf = input;
    char *out_buf = output;
    size_t in_bytes_left = input_len;
    size_t out_bytes_left = output_len;

    size_t res = iconv(cd, &in_buf, &in_bytes_left, &out_buf, &out_bytes_left);
    if (res == (size_t)-1) {
        perror("iconv");
    }

    iconv_close(cd);
    return output_len - out_bytes_left;
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
    char leftover[BUFFER_SIZE] = {0};  // остаток неполной строки
    int lines_printed = 0, paused = 0;
    int header_parsed = 0;
    char *body_start;

    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        int maxfd = sockfd > STDIN_FILENO ? sockfd : STDIN_FILENO;

        int activity = select(maxfd + 1, &read_fds, NULL, NULL, NULL);
        if (activity < 0) error("select");

        if (FD_ISSET(sockfd, &read_fds) && !paused) {
            int n = read(sockfd, buffer, sizeof(buffer));
            if (n <= 0) break;

            char *data = buffer;
            int data_len = n;

            if (!header_parsed) {
                char *header_end = strstr(buffer, "\r\n\r\n");
                if (header_end) {
                    header_parsed = 1;
                    data = header_end + 4;
                    data_len = n - (data - buffer);
                } else {
                    continue;  // ждем конец заголовков
                }
            }

            // Перевод кодировки
            char utf8_buffer[BUFFER_SIZE * 2];
            size_t utf8_len = convert_encoding(data, data_len, utf8_buffer, sizeof(utf8_buffer));
            utf8_buffer[utf8_len] = '\0';

            // Добавим остаток от предыдущего раза
            char combined[BUFFER_SIZE * 3];
            snprintf(combined, sizeof(combined), "%s%s", leftover, utf8_buffer);

            // Построчный вывод
            char *line = strtok(combined, "\n");
            leftover[0] = '\0';

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

            // Сохраняем, если что-то осталось после последнего \n
            if (line == NULL) {
                int len = strlen(combined);
                if (len > 0 && combined[len - 1] != '\n') {
                    strncpy(leftover, combined + (len - (utf8_len - (utf8_buffer + utf8_len - data_len))), sizeof(leftover) - 1);
                }
            }
        }

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
