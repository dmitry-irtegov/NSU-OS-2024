#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/select.h>
#include <termios.h>

#define LINES_PER_SCREEN 25

void get_host_path_lengths(const char* url, size_t* host_len, size_t* path_len) {
    const char* start = strstr(url, "://");
    if (start) {
        start += 3;
    }
    else {
        start = url;
    }

    const char* path_start = strchr(start, '/');
    if (path_start) {
        *host_len = strlen(start) - strlen(path_start);
        *path_len = strlen(path_start);
    }
    else {
        *host_len = strlen(start);
        *path_len = 1;
    }
}

void parse_url(char* url, char* host, char* path) {
    char* start = strstr(url, "://");
    if (start) {
        start += 3;
    }
    else {
        start = url;
    }

    char* path_start = strchr(start, '/');
    if (path_start) {
        strncpy(host, start, path_start - start);
        host[path_start - start] = '\0';
        strcpy(path, path_start);
    }
    else {
        strcpy(host, start);
        strcpy(path, "/");
    }
}

int connect_to_server(const char* host, int port) {
    int sockfd;
    struct sockaddr_in server_addr;
    struct hostent* server;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server = gethostbyname(host);
    if (server == NULL) {
        fprintf(stderr, "Error: No such host\n");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    server_addr.sin_port = htons(port);

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr))) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    return sockfd;
}

void set_non_canonical_mode() {
    struct termios ttystate;
    tcgetattr(STDIN_FILENO, &ttystate);
    ttystate.c_lflag &= ~(ICANON | ECHO);
    ttystate.c_cc[VMIN] = 1;
    tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
}

void restore_canonical_mode() {
    struct termios ttystate;
    tcgetattr(STDIN_FILENO, &ttystate);
    ttystate.c_lflag |= (ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <URL>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    size_t host_len, path_len;

    get_host_path_lengths(argv[1], &host_len, &path_len);

    char host[host_len+1];
    char path[path_len+1];

    parse_url(argv[1], host, path);

    int port = 80;
    int sockfd = connect_to_server(host, port);

    char request[host_len+path_len+50];
    snprintf(request, sizeof(request), "GET %s HTTP/1.0\r\nHost: %s\r\nConnection: close\r\n\r\n", path, host);
    if (write(sockfd, request, strlen(request)) < 0) {
        perror("Write failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    set_non_canonical_mode();

    char buffer[BUFSIZ];
    int lines_printed = 0;
    fd_set read_fds;
    int max_fd = sockfd > STDIN_FILENO ? sockfd : STDIN_FILENO;
    int connection_closed = 0;

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);
        FD_SET(STDIN_FILENO, &read_fds);

        int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("Select error");
            break;
        }

        if (FD_ISSET(sockfd, &read_fds)) {
            char buffer[BUFSIZ];
            char buffer_for_line[BUFSIZ];

            int bytes_read = read(sockfd, buffer, BUFSIZ);
            if (bytes_read <= 0) {
                connection_closed = 1;
                break;
            }
            char* end_buffer = buffer + bytes_read;
            char* temp_buffer = buffer;
            char* line_for_print = buffer_for_line;
            char* line_for_print_start = line_for_print;

            while (temp_buffer < end_buffer) {
                if (*temp_buffer == '\n') {
                    *line_for_print = '\0';
                    printf("%s\n", line_for_print_start);
                    line_for_print = line_for_print_start;
                    lines_printed++;
                }
                else {
                    *line_for_print = *temp_buffer;
                    line_for_print++;
                }

                temp_buffer++;

                if (strlen(line_for_print_start) > 0 && temp_buffer == end_buffer) {
                    *line_for_print = '\0';
                    printf("%s", line_for_print_start);
                }
                if (lines_printed >= LINES_PER_SCREEN) {
                    fprintf(stderr, "Press space to scroll down...\n");
                    while (1) {
                        char input;
                        if (read(STDIN_FILENO, &input, 1) > 0 && input == ' ') {
                            break;
                        }
                    }
                    lines_printed = 0;
                }
            }
        }

        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            char ch;
            if (read(STDIN_FILENO, &ch, 1) > 0 && ch == ' ') {
                lines_printed = 0;
            }
        }
    }

    if (connection_closed) {
        fprintf(stderr, "\nConnection closed by server.\n");
    }

    restore_canonical_mode();
    close(sockfd);
    return 0;
}
