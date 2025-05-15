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

void to_lowercase(char *str) {
    for (; *str; ++str)
        *str = tolower(*str);
}

void get_charset_from_headers(const char *headers, char *charset, size_t max_len) {
    const char *ct = strcasestr(headers, "Content-Type:");
    if (ct) {
        const char *charset_pos = strcasestr(ct, "charset=");
        if (charset_pos) {
            charset_pos += 8; // skip "charset="
            const char *end = strpbrk(charset_pos, ";\r\n");
            if (!end) end = charset_pos + strlen(charset_pos);
            size_t len = end - charset_pos;
            if (len >= max_len) len = max_len - 1;
            strncpy(charset, charset_pos, len);
            charset[len] = '\0';
            to_lowercase(charset);
            return;
        }
    }
    strcpy(charset, "utf-8"); // default fallback
}

void convert_encoding(const char *from_charset, const char *input, size_t input_len, char *output, size_t output_len) {
    iconv_t cd = iconv_open("UTF-8", from_charset);
    if (cd == (iconv_t)(-1)) {
        perror("iconv_open failed, using fallback copy");
        strncpy(output, input, output_len - 1);
        output[output_len - 1] = '\0';
        return;
    }

    const char *in_buf = input;
    char *out_buf = output;
    size_t in_bytes_left = input_len;
    size_t out_bytes_left = output_len;

    size_t res = iconv(cd, (char**)&in_buf, &in_bytes_left, &out_buf, &out_bytes_left);
    if (res == (size_t)(-1)) {
        perror("iconv failed");
    }

    *out_buf = '\0';
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
    int headers_parsed = 0;
    char headers[BUFFER_SIZE * 2] = "";
    char charset[64] = "utf-8"; // default

    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        int maxfd = sockfd > STDIN_FILENO ? sockfd : STDIN_FILENO;

        int activity = select(maxfd + 1, &read_fds, NULL, NULL, NULL);
        if (activity < 0) error("select");

        if (FD_ISSET(sockfd, &read_fds) && !paused) {
            int n = read(sockfd, buffer, sizeof(buffer) - 1);
            if (n <= 0) break;
            buffer[n] = '\0';

            const char *body = buffer;

            if (!headers_parsed) {
                strncat(headers, buffer, sizeof(headers) - strlen(headers) - 1);
                char *header_end = strstr(headers, "\r\n\r\n");
                if (header_end) {
                    headers_parsed = 1;
                    header_end += 4; // skip past \r\n\r\n
                    get_charset_from_headers(headers, charset, sizeof(charset));
                    body = header_end;
                    n = strlen(body); // update size to only body
                } else {
                    continue; // keep reading headers
                }
            }

            char output_buffer[BUFFER_SIZE * 2];
            convert_encoding(charset, body, n, output_buffer, sizeof(output_buffer));

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
