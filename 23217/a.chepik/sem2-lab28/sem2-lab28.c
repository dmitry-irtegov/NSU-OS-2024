#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/select.h>
#include <termios.h>

#define MY_BUFSIZ 131072

int ttyfd = STDIN_FILENO;
struct termios prev_tty;

void exit_tcsetattr() {
    if (tcsetattr(ttyfd, TCSAFLUSH, &prev_tty) == -1) {
        printf("tcsetattr() failed.\n");
    }
}

char* url_host, * url_path;

void my_free() {
    free(url_host);
    free(url_path);
}

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("URL is missing from argv.\n");
        exit(-1);
    }

    if (tcgetattr(ttyfd, &prev_tty) == -1) {
        printf("tcgetattr() failed.\n");
        exit(-1);
    }

    struct termios now_tty = prev_tty;
    now_tty.c_lflag &= ~ICANON;
    now_tty.c_lflag &= ~ECHO;
    now_tty.c_cc[VMIN] = 1;

    if (tcsetattr(ttyfd, TCSAFLUSH, &now_tty) == -1) {
        printf("tcsetattr() failed.\n");
        exit(-1);
    }

    if (atexit(exit_tcsetattr) != 0) {
        printf("atexit() failed.\n");

        if (tcsetattr(ttyfd, TCSAFLUSH, &prev_tty) == -1) {
            printf("tcsetattr() failed.\n");
        }

        exit(-1);
    }

    char* url = argv[1];

    if (!strstr(url, "http") || !strstr(url, "://")) {
        printf("Invalid URL format.\n");
        exit(-1);
    }

    url_host = (char*)malloc(strlen(url) * sizeof(char));
    url_path = (char*)malloc(strlen(url) * sizeof(char));

    if (atexit(my_free) != 0) {
        printf("atexit() failed.\n");
        exit(-1);
    }

    char* url_host_str = strstr(url, "://") + 3; // 3 = len("://")
    char* url_path_str = strstr(url_host_str, "/");

    if (url_path_str != NULL) {
        strcpy(url_path, url_path_str);
        url_host_str[url_path_str - url_host_str] = 0;
    }

    else {
        strcpy(url_path, "/");
    }

    strcpy(url_host, url_host_str);

    struct hostent* server_host = gethostbyname(url_host);
    if (server_host == NULL) {
        printf("gethostbyname() failed.");
        exit(-1);
    }

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_fd == -1) {
        printf("server_host socket() failed.\n");
        exit(-1);
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    memcpy(&addr.sin_addr.s_addr, server_host->h_addr, server_host->h_length);

    if (connect(socket_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        printf("connect() failed.\n");
        close(socket_fd);
        exit(-1);
    }

    int bytes_write = strlen(url_host) + strlen(url_path) + strlen("GET %s HTTP/1.0\r\nHost: %s\r\nConnection: close\r\n\r\n");
    char* get_request = (char*)malloc(bytes_write * sizeof(char));
    snprintf(get_request, bytes_write, "GET %s HTTP/1.0\r\nHost: %s\r\nConnection: close\r\n\r\n", url_path, url_host);
    write(socket_fd, get_request, strlen(get_request));
    free(get_request);

    fd_set read_fds;

    char buffer[MY_BUFSIZ];
    int line_count = 0;
    int flag = 1;
    int end_for_space = 0;
    ssize_t bytes_read = 0;
    off_t  buffer_offset = 0;

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        FD_SET(socket_fd, &read_fds);

        int select_result = select(socket_fd + 1, &read_fds, NULL, NULL, NULL);

        if (select_result == -1) {
            printf("select() failed.\n");
            close(socket_fd);
            exit(-1);
        }

        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            char symbol;
            bytes_read = read(STDIN_FILENO, &symbol, 1);

            if (bytes_read == 1 && symbol == ' ' && flag == 0) {
                flag = 1;
                line_count = 0;

                if (end_for_space == 1) {
                    char* ptr = buffer;
                    char* line_start = buffer;
                    char* end_buffer = buffer + buffer_offset;

                    while (ptr < end_buffer) {
                        if (*ptr == '\n') {
                            if (write(STDOUT_FILENO, line_start, ptr - line_start + 1) == -1) {
                                printf("write() failed.\n");
                                close(socket_fd);
                                exit(-1);
                            }

                            line_start = ptr + 1;
                            buffer_offset -= ptr - line_start + 1;

                            line_count++;
                            if (line_count > 25) {
                                flag = 0;
                                break;
                            }
                        }

                        ptr++;
                    }

                    if (line_start == end_buffer) {
                        break;
                    }

                    if (flag == 1) {
                        if (write(STDOUT_FILENO, line_start, ptr - line_start) == -1) {
                            printf("write() failed.\n");
                            close(socket_fd);
                            exit(-1);
                        }

                        if (write(STDOUT_FILENO, "\n", 1) == -1) {
                            printf("write() failed.\n");
                            close(socket_fd);
                            exit(-1);
                        }

                        break;
                    }

                    else {
                        printf("Press space to output data.\n");
                        memmove(buffer, line_start, buffer_offset);
                    }
                }
            }

            else if (bytes_read < 0) {
                printf("read() from stdin failed.\n");
            }
        }

        if (FD_ISSET(socket_fd, &read_fds)) {
            bytes_read = read(socket_fd, buffer + buffer_offset, MY_BUFSIZ - buffer_offset);

            if (bytes_read < 0) {
                printf("read() from socket failed.\n");
                close(socket_fd);
                exit(-1);
            }

            buffer_offset += bytes_read;

            if (flag == 1) {
                char* ptr = buffer;
                char* line_start = buffer;
                char* end_buffer = buffer + buffer_offset;

                while (ptr < end_buffer) {
                    if (*ptr == '\n') {
                        if (write(STDOUT_FILENO, line_start, ptr - line_start + 1) == -1) {
                            printf("write() failed.\n");
                            close(socket_fd);
                            exit(-1);
                        }

                        line_start = ptr + 1;
                        buffer_offset -= ptr - line_start + 1;

                        line_count++;
                        if (line_count > 25) {
                            flag = 0;
                            break;
                        }
                    }

                    ptr++;
                }

                if (line_start < end_buffer) {
                    if (flag == 0) {
                        printf("Press space to output data.\n");
                    }

                    memmove(buffer, line_start, buffer_offset);
                }
            }

            if (bytes_read == 0) {
                if (buffer_offset == 0) {
                    break;
                }

                end_for_space = 1;
            }
        }
    }

    close(socket_fd);

    exit(0);
}
