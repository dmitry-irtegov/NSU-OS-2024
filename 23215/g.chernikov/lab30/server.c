#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <ctype.h>

#define BUFFER_SIZE 8
char *socket_path = "socket";

int main() {

    int server_socket, client_socket;
    struct sockaddr_un server_addr;
    char buffer[BUFFER_SIZE + 1];

    server_socket = socket(PF_UNIX, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("socket creation failure");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, socket_path, sizeof(server_addr.sun_path) - 1);

    unlink(socket_path);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("socket binding failure");
        close(server_socket);
        exit(1);
    }

    if (listen(server_socket, 1) < 0) {
        perror("listening failure");
        close(server_socket);
        exit(1);
    }

    printf("server is listening in %s\n", socket_path);

    client_socket = accept(server_socket, NULL, NULL);
    if (client_socket < 0) {
        perror("acception failure");
        close(server_socket);
        exit(1);
    }
    printf("client is connected\n");

    while (1) {

        ssize_t bytes_read = read(client_socket, buffer, BUFFER_SIZE);
        if (bytes_read <= 0) {
            break;
        }
    
        buffer[bytes_read] = '\0';

        for (int i = 0; i < bytes_read; i++) {
            buffer[i] = toupper((unsigned char)buffer[i]);
        }

        printf("received: %s\n", buffer);
    }

    printf("client is disconnected\n");

    close(client_socket);
    close(server_socket);
    unlink(socket_path);

    return 0;
}