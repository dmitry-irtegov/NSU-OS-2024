#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/un.h>
#include <ctype.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <stdbool.h>

#define BUFFER_SIZE BUFSIZ
#define MAX_CLIENTS 10
char* path = "socket";
bool endp = true;

void handle_sigint(int sig) {
    endp = false;
}

int main() {
    struct sockaddr_un addr;
    int server, client;
    ssize_t bytes_read;
    char received[BUFFER_SIZE];
    fd_set read_fds;
    int max_fd, clients[MAX_CLIENTS] = {0};
    int i;

    signal(SIGINT, handle_sigint);
    signal(SIGTSTP, handle_sigint);

    server = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server == -1) {
        perror("error creating socket");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);

    if (bind(server, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        close(server);
        perror("bind error");
        exit(EXIT_FAILURE);
    }

    if (listen(server, MAX_CLIENTS) == -1) {
        close(server);
        unlink(path);
        perror("listen error");
        exit(EXIT_FAILURE);
    }
    while (endp) {
        FD_ZERO(&read_fds);
        FD_SET(server, &read_fds);
        max_fd = server;

        for (i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i] > 0) {
                FD_SET(clients[i], &read_fds);
                if (clients[i] > max_fd) {
                    max_fd = clients[i];
                }
            }
        }

        int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if (activity < 0) {
            if (errno != EINTR) {
                perror("select error");
                break;
            }
            continue;
        }

        if (FD_ISSET(server, &read_fds)) {
            client = accept(server, NULL, NULL);
            if (client == -1) {
                perror("accept error");
                continue;
            }

            for (i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i] == 0) {
                    clients[i] = client;
                    break;
                }
            }

            if (i == MAX_CLIENTS) {
                printf("Too many clients connected\n");
                close(client);
            }
        }

        for (i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i] > 0 && FD_ISSET(clients[i], &read_fds)) {
                bytes_read = read(clients[i], received, BUFFER_SIZE);
                if (bytes_read <= 0) {
                    close(clients[i]);
                    clients[i] = 0;
                } else {
                    for (int j = 0; j < bytes_read; j++) {
                        received[j] = toupper(received[j]);
                    }
                    write(1, received, bytes_read);
                }
            }
        }
    }

    for (i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] > 0) {
            close(clients[i]);
        }
    }
    close(server);
    unlink(path);
    exit(EXIT_SUCCESS);
}