#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <ctype.h>

#define SOCKET_PATH "unix_sock"
#define BUFFER_SIZE 128

void handle_client(int client_fd) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    while ((bytes_read = read(client_fd, buffer, BUFFER_SIZE)) > 0) {
        for (int i = 0; i < bytes_read; i++) {
            if (buffer[i] >= 'a' && buffer[i] <= 'z') {
                buffer[i] -= 32;
            }
        }
        write(STDOUT_FILENO, buffer, bytes_read);
    }
    printf("\nClient disconnected.\n");
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_un addr;

    if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("Socket creating error");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    unlink(SOCKET_PATH);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("Socket binding error");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) == -1) {
        perror("Putting socket into idle mode error");
        close(server_fd);
        unlink(SOCKET_PATH);
        exit(EXIT_FAILURE);
    }

    printf("Server started. Waiting for connection...\n");

    if ((client_fd = accept(server_fd, NULL, NULL)) == -1) {
        perror("Accepting connection error");
        close(server_fd);
        unlink(SOCKET_PATH);
        exit(EXIT_FAILURE);
    }

    printf("Client connected.\n");

    handle_client(client_fd);

    close(client_fd);
    close(server_fd);
    unlink(SOCKET_PATH);

    printf("Server shut down.\n");
    return EXIT_SUCCESS;
}
