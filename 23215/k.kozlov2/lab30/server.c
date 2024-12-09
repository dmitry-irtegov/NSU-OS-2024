#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCKET_NAME "/tmp/unix_domain_socket"
#define BUFFER_SIZE 20

int main(int argc, char *argv[]) {
    struct sockaddr_un addr;
    int connection_socket;
    int data_socket;
    int bytes_read;
    char buffer[BUFFER_SIZE];

    unlink(SOCKET_NAME);

    connection_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (connection_socket == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(addr));

    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_NAME, sizeof(addr.sun_path) - 1);

    if (bind(connection_socket, (const struct sockaddr *) &addr, sizeof(addr)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(connection_socket, 20) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    data_socket = accept(connection_socket, NULL, NULL);
    if (data_socket == -1) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    while ((bytes_read = read(data_socket, buffer, BUFFER_SIZE)) > 0) {
        for (int i = 0; i < bytes_read; i++) {
            buffer[i] = toupper(buffer[i]);
        }
        write(1, buffer, bytes_read);
    }

    close(data_socket);
    close(connection_socket);

    unlink(SOCKET_NAME);

    exit(EXIT_SUCCESS);
}
