#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCK_PATH "socket.sock"

int main() {
    int client_fd;
    if ((client_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("Error with socket function");
        exit(-1);
    }
    printf("Created socket.\n");
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCK_PATH, sizeof(addr.sun_path) - 1);
    if (connect(client_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("Error with connect");
        close(client_fd);
        exit(-1);
    }
    printf("Connected to the server.\n");
    char msg[BUFSIZ];
    while (fgets(msg, BUFSIZ, stdin)) {
        if (write(client_fd, msg, strlen(msg)) == -1) {
            perror("Error with write");
            close(client_fd);
            exit(-1);
        }
    }
    close(client_fd);
    exit(0);
}