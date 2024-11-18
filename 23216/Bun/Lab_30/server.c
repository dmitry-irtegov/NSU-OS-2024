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
#define SOCKET_PATH "/tmp/socket"

// function to convert a string to uppercase
void to_upper(char *str) {
    while (*str) {
        *str = toupper((unsigned char)*str);
        str++;
    }
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_un server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    

    // create a socket
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    // set up the server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    // remove the socket file if it already exists
    unlink(SOCKET_PATH);
    
    // bind the socket to the address
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Failed to bind socket");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // start listening on the socket
    if (listen(server_fd, 1) == -1) {
        perror("Failed to listen on socket");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server is running and listening on %s\n", SOCKET_PATH);

    // accept a connection from a client
    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client_fd == -1) {
        perror("Failed to accept connection");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Client connected\n");

    // read data from the client
    ssize_t bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1);
    if (bytes_read == -1) {
        perror("Failed to read data from client");
        close(client_fd);
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    buffer[bytes_read] = '\0';

    // string to uppercase
    to_upper(buffer);

    // print the string
    printf("Transformed string: %s\n", buffer);

    // close the client and server sockets
    close(client_fd);
    close(server_fd);

    // remove the socket file
    unlink(SOCKET_PATH);

    exit(EXIT_SUCCESS);
}