#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCKET_NAME "my_socket"
#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    int read_socket;
    int data_socket;


    if ((read_socket = socket(AF_UNIX, SOCK_STREAM, 0))== -1) {
        perror("socket failed.");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_NAME, sizeof(addr.sun_path) - 1);

    if (bind(read_socket, (const struct sockaddr *) &addr, sizeof(addr)) == -1) {
        perror("bind failed.");
        close(read_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(read_socket, 1) == -1) {
        perror("listen failed.");
        close(read_socket);
        unlink(SOCKET_NAME);
        exit(EXIT_FAILURE);
    }

    if ((data_socket = accept(read_socket, NULL, NULL)) == -1) {
        perror("accept failed.");
        close(data_socket);
        close(read_socket);
        unlink(SOCKET_NAME);
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    ssize_t readBytes;
    while ((readBytes = read(data_socket, buffer, BUFFER_SIZE)) > 0) {
        for (int i = 0; i < readBytes; i++) {
            buffer[i] = toupper(buffer[i]);
        }
        write(STDOUT_FILENO, buffer, readBytes);
    }

    close(data_socket);
    close(read_socket);
    unlink(SOCKET_NAME);
    
    return 0;
}