#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCKET_PATH "unix_socket"
#define BUFFER_SIZE 256

void to_uppercase(char *str) {
    for (int i = 0; str[i]; i++) {
        str[i] = toupper((unsigned char)str[i]);
    }
}

// Функция для инициализации сокета
int initialize_socket(const char *path, int *server_fd) {
    struct sockaddr_un server_addr;

    // Создание сокета
    *server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (*server_fd == -1) return -1;

    // Удаление старого файла сокета
    unlink(path);

    // Настройка адреса сервера
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, path, sizeof(server_addr.sun_path) - 1);

    // Привязка сокета
    if (bind(*server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        return -2;
    }

    // Прослушивание входящих соединений
    if (listen(*server_fd, 5) == -1) {
        return -3;
    }

    return 0;
}

// Функция для обработки соединения
int handle_connection(int client_fd) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    while ((bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1)) > 0) {
        buffer[bytes_read] = '\0';
        to_uppercase(buffer);
        printf("Получено и преобразовано: %s\n", buffer);
    }

    if (bytes_read == -1) return -1; // Ошибка чтения
    return 0;
}

int main() {
    int server_fd, client_fd, result;

    // Инициализация сокета
    result = initialize_socket(SOCKET_PATH, &server_fd);
    if (result != 0) {
        if (result == -1) {
            fprintf(stderr, "Ошибка: не удалось создать сокет.\n");
        } else if (result == -2) {
            fprintf(stderr, "Ошибка: не удалось привязать сокет. Возможно, сервер уже работает.\n");
        } else if (result == -3) {
            fprintf(stderr, "Ошибка: не удалось настроить прослушивание соединений.\n");
        }
        exit(EXIT_FAILURE);
    }

    printf("Сервер слушает соединения на %s\n", SOCKET_PATH);

    // Принятие соединения
    client_fd = accept(server_fd, NULL, NULL);
    if (client_fd == -1) {
        perror("Ошибка: не удалось принять соединение");
        close(server_fd);
        unlink(SOCKET_PATH);
        exit(EXIT_FAILURE);
    }

    // Обработка соединения
    result = handle_connection(client_fd);
    if (result == -1) {
        fprintf(stderr, "Ошибка: не удалось прочитать данные от клиента.\n");
    }

    // Закрытие соединений
    close(client_fd);
    close(server_fd);
    unlink(SOCKET_PATH);

    printf("Сервер завершил работу.\n");

    return 0;
}

