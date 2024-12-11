#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "example_socket"

int main() {
    char* message = "uGaBuGa";
    int sock_fd;
    struct sockaddr_un address;
    ssize_t num_written;

    if ((sock_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sun_family = AF_UNIX;
    strncpy(address.sun_path, SOCKET_PATH, sizeof(address.sun_path));

    if (connect(sock_fd, (struct sockaddr *)&address, sizeof(address)) == -1) {
        perror("connect failed");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    size_t len = strlen(message);
    num_written = write(sock_fd, message, len+1);
    if (num_written == -1) {
        perror("write failed");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }
    close(sock_fd);
    exit(EXIT_SUCCESS);
}