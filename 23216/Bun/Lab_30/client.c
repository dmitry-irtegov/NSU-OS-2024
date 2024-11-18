#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/socket"
#define BUFFER_SIZE 1024

int main() {
    int client_fd;
    struct sockaddr_un server_addr;
    char buffer[BUFFER_SIZE];

    // create a socket
    client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_fd == -1) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    // set up the server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    // connect to the server
    if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Failed to connect to server");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    printf("Enter a string to send to the server: ");
     if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
        perror("Failed to read the string");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    // send the string to the server
    if (write(client_fd, buffer, strlen(buffer)) == -1) {
        perror("Failed to send data to server");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    // close the socket
    close(client_fd);

    exit(EXIT_SUCCESS);
}