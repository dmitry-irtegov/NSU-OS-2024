#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <stdint.h>

#define SOCKET_NAME "sckt"
#define BUFFER_SIZE 100

int main(void) {
    int client_socket;
    struct sockaddr_un server_addr;
    char buffer[BUFFER_SIZE];

    client_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_NAME, sizeof(server_addr.sun_path) - 1);

    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Connect failed");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    while (fgets(buffer, BUFFER_SIZE, stdin) != NULL) {
        size_t msg_size = strlen(buffer);
        
        for (size_t i = 0; i < msg_size; i++) {
            if (write(client_socket, &buffer[i], 1) != 1) {
                perror("Failed to send message byte");
                break;
            }
        }
    }

    close(client_socket);
    return 0;
}