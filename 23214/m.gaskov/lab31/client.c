#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <signal.h>

#define SOCKET_PATH "unix_socket"
#define BUFFER_SIZE 5

void sigpipe_handler(int signo) {
    fprintf(stderr, "Received SIGPIPE (signal %d)\n", signo);
    exit(EXIT_FAILURE);
}

int main() {
    int fd;
    struct sockaddr_un addr;

    signal(SIGPIPE, sigpipe_handler);

    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    if (strlen(SOCKET_PATH) >= sizeof(addr.sun_path)) {
        fprintf(stderr, "Socket path is very long.");
        exit(EXIT_FAILURE);
    }
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path));

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("connect");
        close(fd);
        exit(EXIT_FAILURE);
    }

    printf("Connected to the server.\n");

    char buffer[BUFFER_SIZE];
    while (fgets(buffer, BUFFER_SIZE, stdin)) {
        if (write(fd, buffer, strlen(buffer)) == -1) {
            perror("write");
            close(fd);
            exit(EXIT_FAILURE);
        }
    }

    close(fd);
    exit(EXIT_SUCCESS);
}
