#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "unix_socket"
#define BUFFER_SIZE 1024

int main() {
    int server_sock, client_sock;
    struct sockaddr_un server_addr;

    server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_sock == -1) {
        perror("Socket creating error");
        return EXIT_FAILURE;
    }

    unlink(SOCKET_PATH);
    memset(&server_addr, 0, sizeof(server_addr));
    if (strlen(SOCKET_PATH) >= sizeof(server_addr.sun_path)) {
        printf("Socket path is too long.");
        return EXIT_FAILURE;
    }
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    if (bind(server_sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
        perror("Bind error");
        close(server_sock);
        return EXIT_FAILURE;
    }

    if (listen(server_sock, 10) == -1) {
        perror("Listen error");
        close(server_sock);
        return EXIT_FAILURE;
    }
    printf("Server is waiting.\n");
    
    client_sock = accept(server_sock, NULL, NULL);
    if (client_sock == -1) {
        perror("Accept error");
        close(server_sock);
        return EXIT_FAILURE;
    }
    printf("Client connected.\n");

    char buffer[BUFFER_SIZE];
    int bytes_read;
    while ((bytes_read = read(client_sock, buffer, BUFFER_SIZE - 1)) > 0) {
        buffer[bytes_read] = '\0';
        for (int i = 0; i < bytes_read; i++) {
            buffer[i] = toupper(buffer[i]);
        }
        write(1, buffer, bytes_read);
    }

    if (bytes_read == -1) {
        perror("Read error");
        return EXIT_FAILURE;
    }
    printf("Client disconnected.\n");

    close(client_sock);
    close(server_sock);
    unlink(SOCKET_PATH);
    return EXIT_SUCCESS;
}