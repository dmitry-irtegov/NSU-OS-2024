#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "./unix_socket"
#define BUFFER_SIZE 1024

void to_uppercase(char *str) {
    for (int i = 0; str[i]; i++) {
        if (str[i] >= 'a' && str[i] <= 'z') {
            str[i] -= 32;
        }
    }
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_un server_addr;
    char buffer[BUFFER_SIZE];

    // Создаем сокет
    if ((server_socket = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Настраиваем адрес сервера
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    // Удаляем старый сокет, если он существует
    unlink(SOCKET_PATH);

    // Привязываем сокет
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Слушаем соединения
    if (listen(server_socket, 5) == -1) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on %s\n", SOCKET_PATH);

    // Принимаем соединение
    if ((client_socket = accept(server_socket, NULL, NULL)) == -1) {
        perror("Accept failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Читаем данные от клиента
    ssize_t bytes_read;
    while ((bytes_read = read(client_socket, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0';
        to_uppercase(buffer);
        printf("Received and converted to uppercase: %s\n", buffer);
    }

    if (bytes_read == -1) {
        perror("Read failed");
    }

    // Закрываем соединения
    close(client_socket);
    close(server_socket);
    unlink(SOCKET_PATH);

    return 0;
}
