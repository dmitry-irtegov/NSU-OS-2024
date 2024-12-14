#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "./unix_socket"
#define BUFFER_SIZE 1024

int main() {
    int client_socket;
    struct sockaddr_un server_addr;
    char buffer[BUFFER_SIZE];

    // Создаем сокет
    if ((client_socket = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Настраиваем адрес сервера
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    // Подключаемся к серверу
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Connect failed");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    printf("Connected to server. Enter text to send (Ctrl+D to quit):\n");

    // Отправляем данные серверу
    while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        if (write(client_socket, buffer, strlen(buffer)) == -1) {
            perror("Write failed");
            close(client_socket);
            exit(EXIT_FAILURE);
        }
    }

    // Закрываем соединение
    close(client_socket);

    return 0;
}
