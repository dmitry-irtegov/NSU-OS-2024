#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/select.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

#define SOCKET_PATH "unix_socket"
#define BACKLOG 5
#define BUFFER_SIZE 256

int is_sign_quit = 0;

void sign_quit(int signo) {
    is_sign_quit = 1;
}

void to_uppercase(char *str) {
    int i;
    for (i = 0; str[i]; i++) {
        str[i] = toupper((unsigned char)str[i]);
    }
}

int main() {
    int listen_fd, conn_fd, max_fd;
    struct sockaddr_un addr;
    fd_set master_set, read_set;
    int i;

    if ((listen_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    if (strlen(SOCKET_PATH) >= sizeof(addr.sun_path)) {
        fprintf(stderr, "Socket path is very long.");
        exit(EXIT_FAILURE);
    }
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path));

    if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("bind");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    signal(SIGINT,  sign_quit);
    signal(SIGTERM, sign_quit);
    signal(SIGHUP,  sign_quit);

    if (listen(listen_fd, BACKLOG) == -1) {
        perror("listen");
        close(listen_fd);
        unlink(SOCKET_PATH);
        exit(EXIT_FAILURE);
    }

    FD_ZERO(&master_set);
    FD_SET(listen_fd, &master_set);
    max_fd = listen_fd;

    int is_err_quit = 0;

    while (1) {
        if (is_err_quit || is_sign_quit) {
            break;
        }
        read_set = master_set;
        int ready_count = select(max_fd + 1, &read_set, NULL, NULL, NULL);
        if (ready_count < 0) {
            if (errno == EINTR) {
                break;
            }
            perror("select");
            is_err_quit = 1;
            continue;
        }
        for (i = 3; i <= max_fd; i++) {
            if (!FD_ISSET(i, &read_set)) {
                continue;
            }
            if (i == listen_fd) {
                if ((conn_fd = accept(listen_fd, NULL, NULL)) == -1) {
                    perror("accept");
                    is_err_quit = 1;
                    break;
                } else {
                    if (conn_fd >= FD_SETSIZE) {
                        printf("New client is not connected: set is full.\n");
                        close(conn_fd);
                    }
                    else {
                        FD_SET(conn_fd, &master_set);
                        if (conn_fd > max_fd) {
                            max_fd = conn_fd;
                        }
                        printf("New client connected: fd=%d\n", conn_fd);
                    }
                }
            } else {
                char buffer[BUFFER_SIZE];
                ssize_t bytes_read = read(i, buffer, sizeof(buffer) - 1);
                if (bytes_read <= 0) {
                    if (bytes_read == -1) {
                        perror("read");
                        is_err_quit = 1;
                        break;
                    }
                    printf("Client disconnected: fd=%d\n", i);
                    close(i);
                    FD_CLR(i, &master_set);
                } else {
                    buffer[bytes_read] = '\0';
                    to_uppercase(buffer);
                    printf("%s", buffer);
                    fflush(stdout);
                }
            }
        }
    }
    for (i = 3; i <= max_fd; i++) {
        if (FD_ISSET(i, &master_set)) {
            close(i);
        }
    }
    unlink(SOCKET_PATH);
    exit(is_err_quit ? EXIT_FAILURE : EXIT_SUCCESS);
}
