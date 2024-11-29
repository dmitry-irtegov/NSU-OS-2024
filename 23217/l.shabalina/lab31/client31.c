#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <errno.h>

#define BUFSIZE 1024

int main() {
    struct sockaddr_un addr;
    int client_socket;

    if ((client_socket = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket creation fail");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    const char *socket_path = "SOCKET";
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);

    if (connect(client_socket, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("connection fail");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    char buf[BUFSIZE];
    ssize_t bytes_read;
    while ((bytes_read = read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
        ssize_t bytes_written = 0;
        while (bytes_written < bytes_read) {
            ssize_t result = write(client_socket, buf + bytes_written, bytes_read - bytes_written);
            if (result == -1) {
                perror("write fail");
                close(client_socket);
                exit(EXIT_FAILURE);
            }
            bytes_written += result;
        }
    }

    if (bytes_read == -1) {
        perror("read fail");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    close(client_socket);
    return 0;
}

