#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <poll.h>
#include <errno.h>
#include <stdint.h>

#define SOCKET_NAME "sckt"
#define BUFFER_SIZE 10
#define MAX_CLIENTS 10
#define BACKLOG_SIZE 20

static int successful_connections = 0;
static int active_clients = 0;

int main() {
    int server_socket;
    struct sockaddr_un server_addr;
    struct pollfd fds[MAX_CLIENTS + 1];
    int nfds = 1;
    char buffer[BUFFER_SIZE];

    server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(1);
    }

    unlink(SOCKET_NAME);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_NAME, sizeof(server_addr.sun_path) - 1);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Bind failed");
        close(server_socket);
        exit(2);
    }

    if (listen(server_socket, BACKLOG_SIZE) == -1) {
        perror("Listen failed");
        close(server_socket);
        exit(3);
    }

    printf("Server is listening...\n");

    memset(fds, 0, sizeof(fds));
    fds[0].fd = server_socket;
    fds[0].events = POLLIN;

    while (1) {
        int ret = poll(fds, nfds, -1);
        if (ret == -1) {
            perror("Poll failed");
            break;
        }

        if (fds[0].revents & POLLIN) {
            int client_socket = accept(server_socket, NULL, NULL);
            if (client_socket == -1) {
                perror("Accept failed");
                continue;
            }

            if (active_clients < MAX_CLIENTS) {
                successful_connections++;
                active_clients++;
                printf("New client connected (total successful: %d)\n", successful_connections);
                
                fds[nfds].fd = client_socket;
                fds[nfds].events = POLLIN;
                nfds++;
            } else {
                printf("Too many clients. Connection rejected. (total successful: %d)\n", successful_connections);
                close(client_socket);
            }
        }

        for (int i = 1; i < nfds; i++) {
            if (fds[i].revents & POLLIN) {
                uint32_t msg_size;
                ssize_t bytes_read = read(fds[i].fd, &msg_size, sizeof(msg_size));
                
                if (bytes_read != sizeof(msg_size)) {
                    printf("Client disconnected\n");
                    close(fds[i].fd);
                    active_clients--;
                    memmove(&fds[i], &fds[i + 1], (nfds - i - 1) * sizeof(struct pollfd));
                    nfds--;
                    i--;
                    continue;
                }

                size_t total_read = 0;
                while (total_read < msg_size) {
                    bytes_read = read(fds[i].fd, buffer, 
                                   (msg_size - total_read < BUFFER_SIZE) ? 
                                   msg_size - total_read : BUFFER_SIZE);
                    
                    if (bytes_read <= 0) {
                        break;
                    }

                    for (int j = 0; j < bytes_read; j++) {
                        buffer[j] = toupper(buffer[j]);
                    }
                    write(1, buffer, bytes_read);
                    
                    total_read += bytes_read;
                }
            }
        }
    }

    for (int i = 0; i < nfds; i++) {
        if (fds[i].fd >= 0) {
            close(fds[i].fd);
        }
    }
    unlink(SOCKET_NAME);
    return 0;
}