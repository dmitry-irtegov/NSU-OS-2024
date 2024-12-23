#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCKET_NAME "my_socket"
#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    int socket_fd;

    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCKET_NAME);    

    if ((socket_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket failed.");
        exit(EXIT_FAILURE);
    }


    if (connect(socket_fd, (const struct sockaddr *) &addr, sizeof(addr)) == -1) {
        perror("connect failed.");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    int readBytes = read(STDIN_FILENO, buffer, BUFFER_SIZE);
    if (readBytes != -1) {
        if (write(socket_fd, buffer, readBytes) == -1) {
            perror("write failed.");
            close(socket_fd);
            exit(EXIT_FAILURE);
        }
    } else {
        perror("read failed.");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    close(socket_fd);
    exit(EXIT_SUCCESS);
}

