#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

char* socket_path = "./socket";

int main() {
    int client_fd;
    struct sockaddr_un server_addr;
    char buffer[BUFSIZ];

    if ((client_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(struct sockaddr_un));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, socket_path, sizeof(server_addr.sun_path) - 1);

    if (connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    printf("Connected to server. Type messages to send:\n");

    while (fgets(buffer, BUFSIZ, stdin) != NULL) {
        if (write(client_fd, buffer, strlen(buffer)) == -1) {
            perror("write");
            close(client_fd);
            exit(EXIT_FAILURE);
        }
    }

    close(client_fd);
    return 0;
}

