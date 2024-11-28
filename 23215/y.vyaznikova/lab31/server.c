#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <poll.h>

#define SOCKET_NAME "sckt"
#define BUFFER_SIZE 10
#define MAX_CLIENTS 10

int main() {
    int server_socket, client_socket, i;
    struct sockaddr_un server_addr;
    struct pollfd fds[MAX_CLIENTS + 1];
    char buffer[BUFFER_SIZE];
    pid_t client_pids[MAX_CLIENTS + 1] = {0};

    server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_NAME, sizeof(server_addr.sun_path) - 1);
    unlink(SOCKET_NAME);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Bind failed");
        close(server_socket);
        exit(2);
    }

    if (listen(server_socket, SOMAXCONN) == -1) {
        perror("Listen failed");
        close(server_socket);
        exit(3);
    }

    printf("Server is listening on %s...\n", SOCKET_NAME);

    fds[0].fd = server_socket;
    fds[0].events = POLLRDNORM;

    for (i = 1; i <= MAX_CLIENTS; i++) {
        fds[i].fd = -1;
    }

    while (1) {
        int ret = poll(fds, MAX_CLIENTS + 1, -1);
        if (ret < 0) {
            perror("Poll failed");
            continue;
        }

        if (fds[0].revents & POLLRDNORM) {
            client_socket = accept(server_socket, NULL, NULL);
            if (client_socket == -1) {
                perror("Accept failed");
                continue;
            }

            pid_t client_pid;
            if (read(client_socket, &client_pid, sizeof(client_pid)) <= 0) {
                perror("Failed to read client PID");
                close(client_socket);
                continue;
            }

            printf("New client connected (fd: %d, PID: %d)\n", client_socket, client_pid);

            for (i = 1; i <= MAX_CLIENTS; i++) {
                if (fds[i].fd == -1) {
                    fds[i].fd = client_socket;
                    fds[i].events = POLLRDNORM;
                    client_pids[i] = client_pid;
                    break;
                }
            }
        }

        for (i = 1; i <= MAX_CLIENTS; i++) {
            if (fds[i].fd != -1 && (fds[i].revents & POLLRDNORM)) {
                ssize_t bytes_read = read(fds[i].fd, buffer, BUFFER_SIZE);
                if (bytes_read <= 0) {
                    printf("Client (fd: %d, PID: %d) disconnected\n", fds[i].fd, client_pids[i]);
                    close(fds[i].fd);
                    fds[i].fd = -1;
                    client_pids[i] = 0;
                } else {
                    buffer[bytes_read] = '\0';
                    printf("Received from client (fd: %d, PID: %d): %s", fds[i].fd, client_pids[i], buffer);
                }
            }
        }
    }

    close(server_socket);
    unlink(SOCKET_NAME);
    return 0;
}