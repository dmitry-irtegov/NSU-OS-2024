#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h> 
#include <sys/select.h>

#define BUFFER_SIZE 1024
#define MAX_CLIENTS 2

const char *socketPath = "./socket_file";
int serverSocket_fd = -1;
int client_fds[MAX_CLIENTS];

void close_clients() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_fds[i] != 0) {
            close(client_fds[i]);
        }
    }
}

void handle_sig(int sig) {
    close_clients();
    if (serverSocket_fd != -1) {
        close(serverSocket_fd);
        unlink(socketPath); 
    }
    exit(0);
}

int main() {
    signal(SIGINT, handle_sig);
    signal(SIGHUP, handle_sig);  

    serverSocket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (serverSocket_fd == -1) {
        perror("Socket creation failed");
        exit(1);
    }

    struct sockaddr_un socket_addr;
    memset(&socket_addr, 0, sizeof(socket_addr));
    socket_addr.sun_family = AF_UNIX;
    strncpy(socket_addr.sun_path, socketPath, sizeof(socket_addr.sun_path) - 1);

    if (bind(serverSocket_fd, (struct sockaddr*)&socket_addr, sizeof(socket_addr)) == -1) {
        perror("Bind failed");
        close(serverSocket_fd);
        exit(1);
    }

    if (listen(serverSocket_fd, 2) == -1) {
        perror("Listen failed");
        close(serverSocket_fd);
        unlink(socketPath);
        exit(1);
    }

    printf("Server listening on %s\n", socketPath);

    fd_set read_fds;
    int max_fd = serverSocket_fd;

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(serverSocket_fd, &read_fds);

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_fds[i] != 0) {
                FD_SET(client_fds[i], &read_fds);
            }
            if (client_fds[i] > max_fd) {
                max_fd = client_fds[i]; 
            }


        }

        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("Select error");
            close_clients();
            close(serverSocket_fd);
            unlink(socketPath);
            exit(1);
        }

        if (FD_ISSET(serverSocket_fd, &read_fds)) {
            int free_place = -1;
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (client_fds[i] == 0) {
                    free_place = i;
                    break;
                }
            }
            if (free_place != -1) {
                int clientSocket_fd = accept(serverSocket_fd, NULL, NULL);
                if (clientSocket_fd == -1) {
                    perror("Accept failed");
                    close_clients();
                    close(serverSocket_fd);
                    unlink(socketPath);
                    exit(1);
                }

                client_fds[free_place] = clientSocket_fd;
                printf("Accepted client %d\n", clientSocket_fd);
            }

        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_fds[i] != 0 && FD_ISSET(client_fds[i], &read_fds)) {
                char buffer[BUFFER_SIZE];
                ssize_t bytes_read = read(client_fds[i], buffer, sizeof(buffer));

                if (bytes_read <= 0) {
                    if (bytes_read == 0) {
                        printf("Client %d disconnected.\n", client_fds[i]);
                    } else {
                        perror("read fail");
                    }
                    close(client_fds[i]);
                    client_fds[i] = 0;
                } else {
                    for (ssize_t j = 0; j < bytes_read; j++) {
                        buffer[j] = toupper(buffer[j]);
                    }
                    printf("%.*s", (int)bytes_read, buffer);
                }
            }
        }
    }
}
