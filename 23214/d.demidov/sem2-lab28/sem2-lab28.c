#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/select.h>
#include <termios.h>

#define BUFSIZE 128
#define BATCH_SIZE 24
#define REQ_SIZE 66
#define CLIENT_NAME "irtegovBrowser"

int parse_url(char *url) {
    if (strlen(url) < 8) {
        return -1;
    }

    if (memcmp(url, "https", 5) == 0) {
        return 2;
    }

    if (memcmp(url, "http://", 7) != 0) {
        return 1;
    }

    return 0;
}

int update_buffer_while_waiting(char *buffer, int *index, ssize_t bytes_read, int socket_fd) {
    size_t remainder = bytes_read - *index;
    if (*index != 0) {
        memmove(buffer, buffer + *index, remainder);
    }
    ssize_t local_bytes_read;
    if (BUFSIZE - remainder - 1 < BATCH_SIZE) {
        local_bytes_read = read(socket_fd, buffer + remainder, BUFSIZE - remainder - 1);
    } else {
        local_bytes_read = read(socket_fd, buffer + remainder, BATCH_SIZE);
    }

    if (local_bytes_read == -1) {
        perror("Error reading from socket");
        return -1;
    }

    buffer[remainder + local_bytes_read] = '\0';
    *index = 0;
    return remainder + local_bytes_read;
}

int main(const int argc, char *argv[]) {
    if (argc != 2) {
        perror("Incorrect arguments, should be only URL");
        exit(EXIT_FAILURE);
    }

    int url_parse_res = parse_url(argv[1]);
    if (url_parse_res == 1) {
        perror("Incorrect URL, should be only http://...");
        exit(EXIT_FAILURE);
    }

    if (url_parse_res == -1) {
        perror("Cannot process host name, should be only URL http://...");
        exit(EXIT_FAILURE);
    }

    if (url_parse_res == 2) {
        perror("Cannot process https, should be only http://...");
        exit(EXIT_FAILURE);
    }

    argv[1] += 7;

    struct addrinfo hints;
    struct addrinfo *res;
    int socket_fd;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (0 != getaddrinfo(argv[1], "80", &hints, &res)) {
        perror("Error getting host info");
        exit(EXIT_FAILURE);
    }

    if ((socket_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    if (connect(socket_fd, res->ai_addr, res->ai_addrlen) == -1) {
        perror("Error connecting to host");
        exit(EXIT_FAILURE);
    }

    if (-1 == send(socket_fd, "GET / HTTP/1.0\r\nUser-Agent: irtegovBrowser\r\nConnection: close\r\n\r\n", REQ_SIZE,
                   0)) {
        perror("Error sending request");
        exit(EXIT_FAILURE);
    }

    struct termios ttystate;
    tcgetattr(STDIN_FILENO, &ttystate);
    ttystate.c_lflag &= ~ICANON;
    ttystate.c_lflag &= ~ECHO;
    ttystate.c_cc[VMIN] = 1;
    tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);

    char *buffer = calloc(BUFSIZE, 1);
    if (buffer == NULL) {
        perror("Error allocating memory for buffer");
        exit(EXIT_FAILURE);
    }

    ssize_t bytesRead;
    int lines_count = 0;
    int skipped_headers = 0;
    int skip_shift = 0;
    int *index = malloc(sizeof(int));

    if (index == NULL) {
        perror("Error allocating memory for index");
        exit(EXIT_FAILURE);
    }

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 500000;

    int prev_read = 0;
    int seychas_rvanyot = 0;

    for (;;) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(socket_fd, &readfds);

        if (-1 == select(socket_fd + 1, &readfds, NULL, NULL, NULL)) {
            perror("Error while select");
            break;
        }

        if (FD_ISSET(socket_fd, &readfds)) {
            bytesRead = read(socket_fd, buffer, BATCH_SIZE);

            if (bytesRead == -1) {
                perror("Error while read");
                exit(EXIT_FAILURE);
            }

            if (bytesRead == 0) {
                break;
            }

            while (skipped_headers == 0) {
                while (memcmp(buffer + skip_shift, "\r\n\r\n", 4) != 0 && skip_shift < bytesRead - 4) {
                    skip_shift++;
                }
                if (skip_shift == bytesRead - 4) {
                    if (bytesRead >= 3 && memcmp(buffer + bytesRead - 3, "\r\n\r", 3) == 0) {
                        memmove(buffer, buffer + bytesRead - 3, 3);
                        bytesRead = 3;
                    } else {
                        bytesRead = 0;
                    }

                    ssize_t local_bytes_read = read(socket_fd, buffer + bytesRead, BUFSIZE - bytesRead - 1);

                    if (local_bytes_read == -1) {
                        perror("Error reading from socket");
                        exit(EXIT_FAILURE);
                    }

                    buffer[bytesRead + local_bytes_read] = '\0';
                    bytesRead += local_bytes_read;
                    skip_shift = 0;
                    continue;
                }
                skipped_headers = 1;
                skip_shift += 4;
            }

            buffer[bytesRead] = '\0';
            *index = skip_shift;
            for (; buffer[*index] != '\0'; (*index)++) {
                printf("%c", buffer[*index]);
                fflush(stdout);
                if (buffer[*index] == '\n') {
                    lines_count++;
                    if (lines_count == 24) {
                        printf("Press space to continue");
                        fflush(stdout);

                        char symbol = 0;

                        while (symbol != ' ') {
                            if (seychas_rvanyot == 0) {
                                prev_read = bytesRead;
                                bytesRead = update_buffer_while_waiting(buffer, index, bytesRead, socket_fd);
                                if (prev_read == bytesRead) {
                                    seychas_rvanyot = 1;
                                }
                                if (skip_shift != 0) {
                                    skip_shift = 0;
                                }
                            }
                            FD_SET(STDIN_FILENO, &readfds);
                            if (seychas_rvanyot == 0) {
                                if (-1 == select(1, &readfds, NULL, NULL, &timeout)) {
                                    perror("Error while select");
                                    exit(EXIT_FAILURE);
                                }
                            } else {
                                FD_SET(STDIN_FILENO, &readfds);
                                if (-1 == select(1, &readfds, NULL, NULL, NULL)) {
                                    perror("Error while select");
                                    exit(EXIT_FAILURE);
                                }
                            }

                            if (FD_ISSET(STDIN_FILENO, &readfds) && read(STDIN_FILENO, &symbol, 1) != 1) {
                                perror("Error while read");
                                exit(EXIT_FAILURE);
                            }
                        }
                        printf("\r                       \r");
                        fflush(stdout);
                        seychas_rvanyot = 0;
                        lines_count = 0;
                    }
                }
            }
            if (skip_shift != 0) {
                skip_shift = 0;
            }
        }
    }

    close(socket_fd);
    freeaddrinfo(res);
    free(buffer);
    free(index);

    struct termios temp_ttystate;
    tcgetattr(STDIN_FILENO, &temp_ttystate);
    temp_ttystate.c_lflag |= ICANON | ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &temp_ttystate);

    return 0;
}
