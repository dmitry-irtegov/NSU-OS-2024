#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCKET_PATH "unix_socket"
#define BUFFER_SIZE 256

// Функция для инициализации и подключения клиента
int connect_to_server(const char *path, int *client_fd) {
    struct sockaddr_un server_addr;

    // Создание сокета
    *client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (*client_fd == -1) return -1;

    // Настройка адреса сервера
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, path, sizeof(server_addr.sun_path) - 1);

    // Подключение к серверу
    if (connect(*client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        return -2;
    }

    return 0;
}

int main() {
    int client_fd, result;

    // Подключение к серверу
    result = connect_to_server(SOCKET_PATH, &client_fd);
    if (result != 0) {
        if (result == -1) {
            fprintf(stderr, "Ошибка: не удалось создать сокет.\n");
        } else if (result == -2) {
            fprintf(stderr, "Ошибка: не удалось подключиться к серверу. Возможно, сервер не запущен.\n");
        }
        exit(EXIT_FAILURE);
    }

    printf("Введите текст для отправки на сервер (или 'exit' для выхода):\n");

    char buffer[BUFFER_SIZE];

    // Ввод данных и отправка на сервер
    while (fgets(buffer, BUFFER_SIZE, stdin)) {
        if (strncmp(buffer, "exit", 4) == 0) {
            break;
        }
        if (write(client_fd, buffer, strlen(buffer)) == -1) {
            perror("Ошибка: не удалось отправить данные");
            break;
        }
    }

    close(client_fd);
    printf("Клиент завершил работу.\n");

    return 0;
}

