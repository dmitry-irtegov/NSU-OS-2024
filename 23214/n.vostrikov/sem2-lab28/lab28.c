
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <termios.h>
#include <sys/socket.h>
#include <sys/time.h>

#define MAX_LINES 25
#define MAX_HOST_LEN 1024
#define MAX_PATH_LEN 1024
#define REQ_BUF_SIZE 1024
#define RESP_BUF_SIZE 2
#define MAX_BUF_SIZE 16

typedef struct {
    char* buf;
    size_t len;
    size_t cap;
    size_t pos;
} Buffer;

void parse_url(const char *url, char *host, char *path) {
    const char *start = url;
    if (strncmp(url, "http://", 7) == 0) {
        start += 7;
    } else if (strstr(url, "://") != NULL) {
        fprintf(stderr, "Only 'http://'\n");
        exit(EXIT_FAILURE);
    }
    const char *slash = strchr(start, '/');
    if (slash) {
        size_t host_len = slash - start;
        strncpy(host, start, host_len);
        host[host_len] = '\0';
        strncpy(path, slash + 1, MAX_PATH_LEN - 1);
        path[MAX_PATH_LEN - 1] = '\0';
    } else {
        strncpy(host, start, MAX_HOST_LEN - 1);
        path[0] = '\0';
    }
    if (host[0] == '\0') {
        fprintf(stderr, "Host is empty in URL\n");
        exit(EXIT_FAILURE);
    }
}

struct in_addr resolve_hostname(const char *host) {
    struct hostent *host_info = gethostbyname(host);
    if (!host_info || !host_info->h_addr_list[0]) {
        perror("Host resolution failed");
        exit(EXIT_FAILURE);
    }
    struct in_addr addr;
    memcpy(&addr.s_addr, host_info->h_addr_list[0], sizeof(addr.s_addr));
    return addr;
}

int connect_to_server(struct in_addr addr, int port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_addr = addr,
        .sin_port = htons(port)
    };

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    return sockfd;
}

void send_http_request(int sockfd, const char *host, const char *path) {
    char request[REQ_BUF_SIZE];
    snprintf(request, sizeof(request), "GET /%s HTTP/1.0\r\nHost: %s\r\nUser-Agent: FatBro\r\nConnection: close\r\n\r\n", path, host);
    if (write(sockfd, request, strlen(request)) < 0) {
        perror("Failed to send request");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
}

void restore_terminal(const struct termios *original) {
    tcsetattr(STDIN_FILENO, TCSANOW, original);
}

void configure_terminal(const struct termios *original_ptr, struct termios *modified) {
    *modified = *original_ptr;
    modified->c_lflag &= ~(ICANON | ECHO);
    modified->c_cc[VMIN] = 1;
    modified->c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, modified);
}

void allocate(Buffer* buffer) {
    buffer->buf = malloc(sizeof(char) * RESP_BUF_SIZE);
    buffer->cap = RESP_BUF_SIZE;
    buffer->len = 0;
    buffer->pos = 0;
}

void receive_and_display_response(int sockfd) {
    Buffer response;
    allocate(&response);
    int count_line = 0;
    int pause = 0;
    int active = 1;
    int headers_skipped = 0;
    int buffer_is_full = 0;
    int temp_buffer_is_full = 0;
    char buffer[RESP_BUF_SIZE];
    int read_bytes = 0;
    struct termios original, modified;
    tcgetattr(STDIN_FILENO, &original);
    configure_terminal(&original, &modified);
    fd_set descriptors;
    while(1) {
        FD_ZERO(&descriptors);
        FD_SET(STDIN_FILENO, &descriptors);

        if(active) {
            FD_SET(sockfd, &descriptors);
        }

        if(select(sockfd + 1, &descriptors, NULL, NULL, NULL) < 0) {
            perror("Select error");
            break;
        }

        if(active && FD_ISSET(sockfd, &descriptors) && !buffer_is_full && (response.len + RESP_BUF_SIZE - 1 <= MAX_BUF_SIZE)) {
            if(!temp_buffer_is_full) read_bytes = recv(sockfd, buffer, RESP_BUF_SIZE - 1, 0);
            if(!read_bytes) {
                active = 0;
            }  
            else {
                buffer[read_bytes] = '\0';
                if(read_bytes + response.len > response.cap) {
                    char* new_buf = realloc(response.buf, response.cap * 2);
                    if(new_buf == NULL) {
                        perror("realloc error");
                        break;
                    }
                    else{
                        response.buf = new_buf;
                        response.cap *= 2;
                    }
                }
                memcpy(response.buf + response.len, buffer, read_bytes);
                temp_buffer_is_full = 0;
                response.len += read_bytes;
                response.buf[response.len] = '\0';
                read_bytes = 0;
            }
        }

        if(pause && FD_ISSET(STDIN_FILENO, &descriptors)) {
            char c;
            read(STDIN_FILENO, &c, 1);
            if(c == ' ') {
                pause = 0;
                count_line = 0;
                fflush(stdout);
            }
        }

        if (!headers_skipped) {
            char *headers_end = strstr(response.buf, "\r\n\r\n");
            if (headers_end) {
                headers_skipped = 1;
                memmove(response.buf, headers_end + 4, sizeof(char) * (response.len - (headers_end - response.buf + 4)));
                response.len -= (headers_end - response.buf + 4);
                buffer_is_full = 0;
            }
            else {
                if(response.len > 3) {
                    memmove(response.buf, response.buf + response.len - 3, 3);
                    response.len = 3;
                    buffer_is_full = 0;
                }
            }
        }

        if(!pause && headers_skipped) {
            int ch_to_print = 0;
            while(ch_to_print < response.len) {
                if (response.buf[ch_to_print++] == '\n') {
                    count_line++;
                }
                if(count_line == MAX_LINES) break;
            }
            if(ch_to_print > 0) {
                write(STDOUT_FILENO, response.buf, ch_to_print);
                memmove(response.buf, response.buf + ch_to_print, sizeof(char) * (response.len - ch_to_print));
                response.len -= ch_to_print;
                buffer_is_full = 0;
                if(count_line == MAX_LINES) {
                    pause = 1;
                    printf("Press space to continue\n");
                    fflush(stdout);
                }
            }
        }
        if (!active && !response.len) break;
    }
    restore_terminal(&original);
    free(response.buf);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        perror("No url");
        return EXIT_FAILURE;
    }

    char host[MAX_HOST_LEN] = {0};
    char path[MAX_PATH_LEN] = {0};
    parse_url(argv[1], host, path);

    printf("Host: %s\n", host);
    printf("Path: /%s\n", path);

    struct in_addr ip = resolve_hostname(host);
    printf("Resolved IP: %s\n", inet_ntoa(ip));

    int sockfd = connect_to_server(ip, 80);
    printf("Connected to server\n");

    send_http_request(sockfd, host, path);
    printf("Request sent\n");
    receive_and_display_response(sockfd);
    close(sockfd);

    return EXIT_SUCCESS;
}