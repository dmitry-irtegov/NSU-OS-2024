#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <termios.h>
#include <signal.h>
#include <iconv.h>

#define PORT 80
#define BUFFER_SIZE 4096
#define LINES_PER_PAGE 25

struct termios orig_termios;

void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void handle_signal(int sig) {
    disable_raw_mode();
    printf("\n[Exiting on signal %d]\n", sig);
    exit(1);
}

void setup_signal_handlers() {
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    signal(SIGHUP, handle_signal);
}

void enable_raw_mode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disable_raw_mode);
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

char *convert_encoding(const char *input, const char *from_charset, const char *to_charset) {
    iconv_t cd = iconv_open(to_charset, from_charset);
    if (cd == (iconv_t)(-1)) {
        perror("iconv_open");
        return NULL;
    }

    size_t inbytesleft = strlen(input);
    size_t outbytesleft = inbytesleft * 4;

    char *output = malloc(outbytesleft + 1);
    if (!output) {
        iconv_close(cd);
        return NULL;
    }

    const char *inbuf = input;  // input не изменяем
    const char **pinbuf = &inbuf; // указатель на константный указатель

    char *outptr = output;

    if (iconv(cd, (char **)pinbuf, &inbytesleft, &outptr, &outbytesleft) == (size_t)-1) {
        perror("iconv");
        free(output);
        iconv_close(cd);
        return NULL;
    }

    *outptr = '\0';
    iconv_close(cd);
    return output;
}

void parse_url(const char *url, char *host, char *path) {
    const char *p = strstr(url, "http://");
    if (p) url += strlen("http://");

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

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s http://host/path\n", argv[0]);
        exit(1);
    }

    setup_signal_handlers();

    char host[256], path[1024];
    parse_url(argv[1], host, path);

    struct hostent *server = gethostbyname(host);
    if (!server) {
        perror("gethostbyname");
        exit(1);
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        exit(1);
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect");
        close(sock);
        exit(1);
    }

    char request[1024];
    int len = snprintf(request, sizeof(request),
                       "GET %s HTTP/1.0\r\nHost: %s\r\nConnection: close\r\n\r\n",
                       path, host);
    if (len >= sizeof(request)) {
        fprintf(stderr, "Request too long!\n");
        exit(1);
    }

    send(sock, request, strlen(request), 0);
    enable_raw_mode();

    char buffer[BUFFER_SIZE];
    int line_count = 0;
    int header_parsed = 0;
    int use_iconv = 0;

    while (1) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);
        FD_SET(STDIN_FILENO, &readfds);

        int maxfd = (sock > STDIN_FILENO ? sock : STDIN_FILENO) + 1;
        int ready = select(maxfd, &readfds, NULL, NULL, NULL);
        if (ready < 0) {
            perror("select");
            break;
        }

        if (FD_ISSET(sock, &readfds)) {
            int bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
            if (bytes <= 0) break;
            buffer[bytes] = '\0';

            char *content = buffer;

            if (!header_parsed) {
                char *header_end = strstr(buffer, "\r\n\r\n");
                if (header_end) {
                    *header_end = '\0';
                    if (strstr(buffer, "charset=windows-1251") || strstr(buffer, "charset=WINDOWS-1251")) {
                        use_iconv = 1;
                    }
                    content = header_end + 4;
                    header_parsed = 1;
                } else {
                    continue;
                }
            }

            char *line = strtok(content, "\n");
            while (line) {
                char *output = use_iconv ? convert_encoding(line, "WINDOWS-1251", "UTF-8") : NULL;
                printf("%s\n", output ? output : line);
                if (output) free(output);

                line_count++;
                if (line_count >= LINES_PER_PAGE) {
                    printf("\n-- Press space to scroll down --\n");
                    fflush(stdout);
                    while (1) {
                        FD_ZERO(&readfds);
                        FD_SET(STDIN_FILENO, &readfds);
                        select(STDIN_FILENO + 1, &readfds, NULL, NULL, NULL);
                        char ch;
                        read(STDIN_FILENO, &ch, 1);
                        if (ch == ' ') {
                            line_count = 0;
                            break;
                        }
                    }
                }

                line = strtok(NULL, "\n");
            }
        }
    }

    close(sock);
    return 0;
}
