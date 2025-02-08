#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCKET_PATH "unix_socket"
#define BUFFER_SIZE 1024

int main() {
    int server_fd, client_fd;
    struct sockaddr_un server_addr;
    char buffer[BUFFER_SIZE];

    // Создание сокета
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Настройка адреса
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    // Удаление старого сокета, если он существует
    unlink(SOCKET_PATH);

    // Привязка сокета
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Прослушивание соединений
    if (listen(server_fd, 1) == -1) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Сервер ожидает соединение...\n");

    // Принятие соединения
    client_fd = accept(server_fd, NULL, NULL);
    if (client_fd == -1) {
        perror("accept");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Чтение данных и преобразование в верхний регистр
    ssize_t num_read;
    while ((num_read = read(client_fd, buffer, BUFFER_SIZE)) > 0) {
        for (ssize_t i = 0; i < num_read; i++) {
            buffer[i] = toupper(buffer[i]);
        }
        buffer[num_read] = '\0';  // Обеспечение корректного вывода
        printf("Получено и преобразовано: %s\n", buffer);
    }

    if (num_read == -1) {
        perror("read");
    }

    if (num_read == 0) {
        printf("Клиент разорвал соединение.\n");
    }

    // Закрытие соединений
    close(client_fd);
    close(server_fd);
    unlink(SOCKET_PATH);
    return 0;
}
