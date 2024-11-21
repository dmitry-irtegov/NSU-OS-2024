#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include "server.h"

extern const struct sockaddr_un sock_addr;
extern const int sock_addr_len;

int main() {
    char* line = NULL;
    size_t len = 0;
    ssize_t bytes_read;
    int socket_fd = socket(PF_UNIX, SOCK_STREAM, 0);
    
    if (socket_fd == -1) {
        fprintf(stderr, "socket() ended with error\n");
        exit(EXIT_FAILURE);
    }

    if (connect(socket_fd, (struct sockaddr*) &sock_addr, sock_addr_len)) {
        fprintf(stderr, "connect() ended with error\n");
        exit(EXIT_FAILURE);
    }
    
    printf("Press ENTER on a empty line to stop the input\n");
    bytes_read = getline(&line, &len, stdin);

    while (bytes_read > 1) {
        if (send(socket_fd, line, bytes_read, 0) == -1) {
            fprintf(stderr, "send() ended with error\n");
            close(socket_fd);
            exit(EXIT_FAILURE);
        }
    
        bytes_read = getline(&line, &len, stdin);
    }

    close(socket_fd);

    exit(EXIT_SUCCESS);
}
