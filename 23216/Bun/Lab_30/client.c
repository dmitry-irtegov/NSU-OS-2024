#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>

#define SOCKET_PATH "/tmp/socket"
#define BUFFER_SIZE 1024

void lostConnection() {
    write(STDERR_FILENO, "Lost server connection\n", strlen("Lost server connection\n") + 1);
    _exit(EXIT_FAILURE);
}

int main() {
    int client_fd;
    struct sockaddr_un server_addr;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    // create a socket
    client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_fd == -1) {
        perror("Failed to create socket\n");
        exit(EXIT_FAILURE);
    }

    // set up the server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    // connect to the server
    if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Failed to connect to server\n");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    signal(SIGPIPE, lostConnection);
    printf("Enter a string to send to the server (type 'exit' to quit):\n");

    while (1) {
        bytes_read = read(STDIN_FILENO, buffer, BUFFER_SIZE);
        if (bytes_read == -1) {
            perror("Failed to read input\n");
            break;
        } else if (bytes_read == 0) {
            fprintf(stderr, "EOF\n");
            break;
        }

        // if you want to exit
        if (strncmp(buffer, "exit", 4) == 0) {
            break;
        }

        // send the string to the server
        if (write(client_fd, buffer, bytes_read) == -1) {
            perror("Failed to send data to server\n");
            close(client_fd);
            exit(EXIT_FAILURE);
        }
    }

    close(client_fd);

    exit(EXIT_SUCCESS);
}
