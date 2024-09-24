#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/lab30socket"
#define BUFFER_SIZE 256

int main() {
    int server_fd, client_fd;
    struct sockaddr_un server_addr;
    char buffer[BUFFER_SIZE];

    if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket error");
        exit(EXIT_FAILURE);
    }

    unlink(SOCKET_PATH);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind error");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) == -1) {
        perror("listen error");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server is waiting to connect...\n");

    if ((client_fd = accept(server_fd, NULL, NULL)) == -1) {
        perror("accept error");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Client is connected. Waiting for text...\n");

    ssize_t bytes_read;
    while ((bytes_read = read(client_fd, buffer, BUFFER_SIZE)) > 0) {
        printf("Text output: ");
        for (int i = 0; i < bytes_read; i++) {
            printf("%c", toupper(buffer[i]));
        }
        printf("\n");
    }

    if (bytes_read == -1) {
        perror("read error");
    }

    printf("Client is disconnected. off server.\n");

    close(client_fd);
    close(server_fd);
    unlink(SOCKET_PATH);

    return 0;
}
