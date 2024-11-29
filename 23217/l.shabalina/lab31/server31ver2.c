#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <errno.h>
#include <ctype.h>
#include <sys/select.h>
#include <signal.h>

#define BUFSIZE 1024
#define MAX_CLIENTS 10

int quit_flag = 0;

void handle_sigint(int sig) {
    quit_flag = 1;
}

int main() {
    struct sockaddr_un addr;
    int server_fd, client_fd;
    fd_set read_fds;
    int max_fd;
    int client_fds[MAX_CLIENTS] = {0};

    if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket creation fail");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    const char *socket_path = "SOCKET";
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind fail");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, MAX_CLIENTS) == -1) {
        perror("listen fail");
        close(server_fd);
        unlink(socket_path);
        exit(EXIT_FAILURE);
    }

    max_fd = server_fd;

    signal(SIGINT, handle_sigint);

    while (1) {
        if (quit_flag) {
            close(server_fd);
            unlink(socket_path);
            exit(EXIT_SUCCESS);
        }

        FD_ZERO(&read_fds);
        FD_SET(server_fd, &read_fds);

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_fds[i] > 0) {
                FD_SET(client_fds[i], &read_fds);
            }
        }

        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                perror("select fail");
                close(server_fd);
                unlink(socket_path);
                exit(EXIT_FAILURE);
            }
        }

        if (FD_ISSET(server_fd, &read_fds)) {
            if ((client_fd = accept(server_fd, NULL, NULL)) == -1) {
                perror("accept fail");
                close(server_fd);
                unlink(socket_path);
                exit(EXIT_FAILURE);
            }

            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (client_fds[i] == 0) {
                    client_fds[i] = client_fd;
                    if (client_fd > max_fd) {
                        max_fd = client_fd;
                    }
                    FD_SET(client_fd, &read_fds); 
                    break;
                }
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_fds[i] > 0 && FD_ISSET(client_fds[i], &read_fds)) {
                char buf[BUFSIZE];
                ssize_t bytes_read;
                if ((bytes_read = read(client_fds[i], buf, sizeof(buf))) > 0) {
                    for (ssize_t j = 0; j < bytes_read; j++) {
                        printf("%c", toupper(buf[j]));
                    }
                } else if (bytes_read == 0) {
                    close(client_fds[i]);
                    client_fds[i] = 0;
                } else {
                    perror("read fail");
                    close(client_fds[i]);
                    client_fds[i] = 0;
                }
            }
        }
    }
    
}

