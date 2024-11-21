#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <ctype.h>

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 1024
#endif
#ifndef SOCKET_PATH
#define SOCKET_PATH "/tmp/socket"
#endif
int main() {
    int server_fd, client_fd;
    struct sockaddr_un server_addr;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    // create a socket
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Failed to create socket\n");
        exit(EXIT_FAILURE);
    }

    // set up the server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    unlink(SOCKET_PATH);
    
    // bind the socket to the address
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Failed to bind socket\n");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // start listening on the socket
    if (listen(server_fd, 1) == -1) {
        perror("Failed to listen on socket\n");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server is running and listening on %s\n", SOCKET_PATH);

    // sccept a connection from a client
    client_fd = accept(server_fd, NULL, NULL);
    if (client_fd == -1) {
        perror("Failed to accept connection\n");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Client connected\n");
    printf("Transformed string: \n");

    // read data from the client
    while ((bytes_read = read(client_fd, buffer, BUFFER_SIZE)) > 0) {
        for (int i = 0; i < bytes_read; i++) {
            buffer[i] = toupper(buffer[i]);
        }
        write(STDOUT_FILENO, buffer, bytes_read);
    }

    if (bytes_read == -1) {
        perror("Failed to read data from client\n");
        close(client_fd);
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    close(client_fd);
    close(server_fd);

    unlink(SOCKET_PATH);

    exit(EXIT_SUCCESS);
}
