#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "example_socket"
#define BUFFER_SIZE 16

int main() {
    int server_fd, client_fd;
    struct sockaddr_un address;
    char buffer[BUFFER_SIZE];
    ssize_t num_read;

    if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sun_family = AF_UNIX;
    strncpy(address.sun_path, SOCKET_PATH, sizeof(address.sun_path));

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == -1) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 1) == -1) {
        perror("listen failed");
        close(server_fd);
        unlink(SOCKET_PATH);
        exit(EXIT_FAILURE);
    }

    printf("Server listen\n");

    if ((client_fd = accept(server_fd, NULL, NULL)) == -1) {
        perror("accept failed");
        close(server_fd);
        unlink(SOCKET_PATH);
        exit(EXIT_FAILURE);
    }

    while ((num_read = read(client_fd, buffer, BUFFER_SIZE-1)) > 0) {
        for (ssize_t i = 0; i < num_read; ++i) {
            buffer[i] = toupper(buffer[i]);
        }
        buffer[num_read] = '\0';
        printf("%s", buffer);
    }
    if (num_read == -1) {
        perror("read failed");
        close(client_fd);
        close(server_fd);
        unlink(SOCKET_PATH);
        exit(EXIT_FAILURE);
    }
    close(client_fd);
    close(server_fd);
    unlink(SOCKET_PATH);
    exit(EXIT_SUCCESS);
}