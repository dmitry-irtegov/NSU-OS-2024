#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCKET_PATH "unix_socket"
#define BUFFER_SIZE 1024

int main() {
    int client_fd;
    struct sockaddr_un server_addr;
    char buffer[BUFFER_SIZE];

    // Создание сокета
    if ((client_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Настройка адреса сервера
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    // Подключение к серверу
    if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    printf("Соединение с сервером установлено. Введите текст для отправки:\n");

    // Отправка данных серверу
    while (fgets(buffer, BUFFER_SIZE, stdin) != NULL) {
        if (write(client_fd, buffer, strlen(buffer)) == -1) {
            perror("write");
            break;
        }
    }

    // Закрытие соединения
    close(client_fd);
    printf("Соединение с сервером закрыто.\n");

    return 0;
}
