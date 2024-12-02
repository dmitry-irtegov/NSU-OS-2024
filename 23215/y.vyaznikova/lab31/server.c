#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <poll.h>
#include <errno.h>

#define SOCKET_NAME "socket"
#define BUFFER_SIZE 10
#define MAX_CLIENTS 10

ssize_t read_n_bytes(int fd, void *buffer, size_t n) {
    size_t bytes_left = n;
    size_t bytes_read = 0;
    char *buf = (char *)buffer;

    while (bytes_left > 0) {
        ssize_t result = read(fd, buf + bytes_read, bytes_left);
        
        if (result == 0) {
            return bytes_read;
        }
        
        if (result < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        
        bytes_read += result;
        bytes_left -= result;
    }
    
    return bytes_read;
}

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

    if (listen(server_socket, 5) == -1) {
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

            if (nfds < MAX_CLIENTS + 1) {
                fds[nfds].fd = client_socket;
                fds[nfds].events = POLLIN;
                nfds++;
                printf("New client connected\n");
            } else {
                printf("Too many clients. Connection rejected.\n");
                close(client_socket);
            }
        }

        for (int i = 1; i < nfds; i++) {
            if (fds[i].revents & POLLIN) {
                uint32_t msg_size;
                ssize_t size_read = read_n_bytes(fds[i].fd, &msg_size, sizeof(msg_size));
                
                if (size_read != sizeof(msg_size)) {
                    printf("Client disconnected\n");
                    close(fds[i].fd);
                    memmove(&fds[i], &fds[i + 1], (nfds - i - 1) * sizeof(struct pollfd));
                    nfds--;
                    i--;
                    continue;
                }

                uint32_t bytes_remaining = msg_size;
                while (bytes_remaining > 0) {
                    size_t to_read = (bytes_remaining > BUFFER_SIZE - 1) ? BUFFER_SIZE - 1 : bytes_remaining;
                    ssize_t bytes_read = read_n_bytes(fds[i].fd, buffer, to_read);
                    
                    if (bytes_read <= 0) {
                        printf("Error reading message content\n");
                        break;
                    }

                    buffer[bytes_read] = '\0';
                    for (int j = 0; j < bytes_read; j++) {
                        buffer[j] = toupper(buffer[j]);
                    }
                    printf("%s", buffer);
                    
                    bytes_remaining -= bytes_read;
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