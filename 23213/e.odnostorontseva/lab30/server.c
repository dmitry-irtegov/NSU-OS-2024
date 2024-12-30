#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <ctype.h>

int main() {
    int server_fd, client_fd;  
    char *socket_path = "./socket";  

    if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket error");
        exit(1);
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("bind error");
        exit(1);
    }

    if (listen(server_fd, 0) == -1) {
        perror("listen error");
        unlink(socket_path);
        exit(1);
    }

    printf("Server is listening...\n");

    if ((client_fd = accept(server_fd, NULL, NULL)) == -1) {
        perror("accept error");
        unlink(socket_path);
        exit(1);
    }

    printf("Client connected.\n");

    char buffer[BUFSIZ];
    ssize_t bytes_read;

    while ((bytes_read = read(client_fd, buffer, BUFSIZ)) > 0) {
        for (int i = 0; i < bytes_read; i++) {
            buffer[i] = toupper(buffer[i]);
        }
        printf("%.*s", (int)bytes_read, buffer);
    }

    if (bytes_read == 0) {
        printf("Client disconnected.\n");
    } else if (bytes_read == -1) {
        perror("read error");
    }


    unlink(socket_path);
    printf("Server stopped.\n");
    exit(0);
}

