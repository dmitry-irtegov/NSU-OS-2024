#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "unix_socket"
#define BUFFER_SIZE 1024

int main() {
    int client_sock;
    struct sockaddr_un server_addr;

    client_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_sock == -1) {
        perror("Socket creating error");
        return EXIT_FAILURE;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    if (strlen(SOCKET_PATH) >= sizeof(server_addr.sun_path)) {
        printf("Socket path is too long.");
        return EXIT_FAILURE;
    }
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    if (connect(client_sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
        perror("Connect error");
        close(client_sock);
        return EXIT_FAILURE;
    }
    printf("Connected with server.\n");

    char buffer[BUFFER_SIZE];
    while (fgets(buffer, BUFFER_SIZE, stdin) != NULL) {
        if (write(client_sock, buffer, strlen(buffer)) == -1) {
            perror("Write error");
            close(client_sock);
            return EXIT_FAILURE;
        }
    }
    printf("Disconnected.\n");

    close(client_sock);
    return EXIT_SUCCESS;
}