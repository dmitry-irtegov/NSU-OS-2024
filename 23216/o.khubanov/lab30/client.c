#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCKET_PATH "unix_socket"
#define BUFFER_SIZE 256

// Функция для инициализации и подключения клиента
int connect_to_server(const char *path) {
    int client_fd;
    struct sockaddr_un server_addr;

    // Создание сокета
    if ((client_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("Ошибка: не удалось создать сокет");
        exit(EXIT_FAILURE);
    }

    // Настройка адреса сервера
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, path, sizeof(server_addr.sun_path) - 1);

    // Подключение к серверу
    if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Ошибка: не удалось подключиться к серверу");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    return client_fd;
}

// Функция для взаимодействия с сервером
void communicate_with_server(int client_fd) {
    char buffer[BUFFER_SIZE];
    printf("Введите текст для отправки на сервер (или 'exit' для выхода):\n");

    while (fgets(buffer, BUFFER_SIZE, stdin)) {
        if (strncmp(buffer, "exit", 4) == 0) {
            break;
        }
        if (write(client_fd, buffer, strlen(buffer)) == -1) {
            perror("Ошибка: не удалось отправить данные");
            break;
        }
    }
}

int main() {
    int client_fd = connect_to_server(SOCKET_PATH);
    communicate_with_server(client_fd);
    close(client_fd);

    return 0;
}

