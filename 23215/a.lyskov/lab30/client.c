#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "unix_sock"
#define BUFFER_SIZE 128

int connect_to_server(const char *path) {
    int client_fd;
    struct sockaddr_un addr;

    if ((client_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("Socket creating error");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);

    if (connect(client_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("Server connection error");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    return client_fd;
}

void send_data(int client_fd) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    printf("Enter text for sending (CTRL+D to exit):\n");

    while ((bytes_read = read(STDIN_FILENO, buffer, BUFFER_SIZE)) > 0) {
        if (write(client_fd, buffer, bytes_read) == -1) {
            perror("Data sending error");
            close(client_fd);
            exit(EXIT_FAILURE);
        }
    }
}

int main() {
    int client_fd = connect_to_server(SOCKET_PATH);
    send_data(client_fd);
    close(client_fd);
    printf("Connection closed.\n");
    return 0;
}
