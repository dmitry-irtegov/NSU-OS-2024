#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <ctype.h>
#include <poll.h>
#include <signal.h>

#define BACKLOG 17

const char* socket_path = "my_socket";

void my_unlink() {
    unlink(socket_path);
    printf("Unlink is done.\n");
}

void handler(int signum) {
    printf("Server interrupted by SIGINT signal.\n");
    exit(-1);
}

int main() {
    int socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (socket_fd == -1) {
        printf("server socket() failed.\n");
        exit(-1);
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));

    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);

    if (bind(socket_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        printf("bind() failed.\n");
        close(socket_fd);
        exit(-1);
    }

    if (atexit(my_unlink) != 0) {
        printf("atexit() failed.\n");
        close(socket_fd);
        exit(-1);
    }

    signal(SIGINT, handler);

    if (listen(socket_fd, 1) != 0) {
        printf("listen() failed.\n");
        close(socket_fd);
        exit(-1);
    }

    nfds_t fds_count = 1;
    struct pollfd fds[BACKLOG + 1];
    memset(fds, 0, sizeof(fds));
    fds[0].fd = socket_fd;
    fds[0].events = POLLIN;

    while (1) {
        int poll_result = poll(fds, fds_count, -1);

        if (poll_result == -1) {
            printf("poll() failed.\n");
            close(socket_fd);
            exit(-1);
        }

        for (int i = 0; i < fds_count; i++) {
            if (fds[i].revents & POLLIN) {
                if (fds[i].fd == socket_fd) {
                    int client_fd = accept(socket_fd, NULL, NULL);

                    if (client_fd == -1) {
                        printf("accept() failed.\n");
                        continue;
                    }

                    if (fds_count == BACKLOG + 1) {
                        close(client_fd);
                    }

                    else {
                        fds[fds_count].fd = client_fd;
                        fds[fds_count++].events = POLLIN;
                    }

                }

                else {
                    char buffer[BUFSIZ];
                    ssize_t bytes_read = read(fds[i].fd, buffer, sizeof(buffer));

                    if (bytes_read > 0) {
                        for (ssize_t k = 0; k < bytes_read; k++) {
                            printf("%c", toupper(buffer[k]));
                        }

                    }

                    else {
                        if (bytes_read < 0) {
                            printf("server read() failed.\n");
                        }

                        close(fds[i].fd);
                        fds[i] = fds[fds_count - 1];
                        i--;
                        fds_count--;
                    }
                }
            }
        }
    }

    exit(0);
}
