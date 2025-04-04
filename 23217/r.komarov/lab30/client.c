#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "./socket"

int main() {
    int client_socket;
    struct sockaddr_un server_addr;

    if ((client_socket = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect failed");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    char buffer[BUFSIZ];
    ssize_t bytes_read;

    while ((bytes_read = read(STDIN_FILENO, buffer, BUFSIZ - 1)) > 0) {
        size_t total_written = 0;

        while (total_written < bytes_read) {
            ssize_t written = write(client_socket, buffer + total_written, bytes_read - total_written);
            if (written == -1) {
                perror("write failed");
                close(client_socket);
                exit(EXIT_FAILURE);
            }
            total_written += written;
        }

        size_t total_read = 0;

        while ((bytes_read = read(client_socket, buffer + total_read, BUFSIZ - 1 - total_read)) > 0) {
            total_read += bytes_read;
            if (buffer[total_read - 1] == '\n' || total_read == BUFSIZ - 1) {
                break;
            }
        }

        if (bytes_read == 0) {
            break;
        } else if (bytes_read == -1) {
            perror("read failed");
            break;
        }

        buffer[total_read] = '\0';
        printf("%s\n", buffer);
    }

    close(client_socket);
    return 0;
}

