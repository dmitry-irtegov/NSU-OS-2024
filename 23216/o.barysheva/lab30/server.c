#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCKET_FILE "unix_socket"
#define MAX_BUFFER 1024

int main() {
    int server_socket, client_socket;
    struct sockaddr_un socket_address;
    char data_buffer[MAX_BUFFER];

    // Создание сокета
    server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creating error");
        return EXIT_FAILURE;
    }

    memset(&socket_address, 0, sizeof(socket_address));
    socket_address.sun_family = AF_UNIX;
    strncpy(socket_address.sun_path, SOCKET_FILE, sizeof(socket_address.sun_path) - 1);

    unlink(SOCKET_FILE);

    if (bind(server_socket, (struct sockaddr*)&socket_address, sizeof(socket_address)) == -1) {
        perror("Bind error");
        close(server_socket);
        return EXIT_FAILURE;
    }

    if (listen(server_socket, 1) == -1) {
        perror("Listen error");
        close(server_socket);
        return EXIT_FAILURE;
    }

    printf("Server is waiting.\n");

    client_socket = accept(server_socket, NULL, NULL);
    if (client_socket == -1) {
        perror("Accept error");
        close(server_socket);
        return EXIT_FAILURE;
    }

    printf("Client connected.\n");

    
    ssize_t bytes_received;
    while ((bytes_received = read(client_socket, data_buffer, MAX_BUFFER - 1)) > 0) {
        for (ssize_t i = 0; i < bytes_received; i++) {
            data_buffer[i] = toupper((unsigned char)data_buffer[i]);
        }
        data_buffer[bytes_received] = '\0';
        printf("Proccesed data: %s\n", data_buffer);
    }

    if (bytes_received == -1) {
        perror("Read error");
    } 
    else {
        printf("Client disconnected.\n");
    }

    // Закрытие сокетов и удаление файла
    close(client_socket);
    close(server_socket);
    unlink(SOCKET_FILE);

    return EXIT_SUCCESS;
}