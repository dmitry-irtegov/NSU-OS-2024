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
int initialize_socket(const char *path) {
    int server_fd;
    struct sockaddr_un server_addr;

    // Создание сокета
    if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("Ошибка: не удалось создать сокет");
        exit(EXIT_FAILURE);
    }

    // Удаление старого файла сокета
    unlink(path);

    // Настройка адреса сервера
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, path, sizeof(server_addr.sun_path) - 1);

    // Привязка сокета
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Ошибка: не удалось привязать сокет");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Прослушивание входящих соединений
    if (listen(server_fd, 5) == -1) {
        perror("Ошибка: не удалось настроить прослушивание");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    return server_fd;
}

// Функция для работы с соединением
void handle_connection(int client_fd) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    while ((bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1)) > 0) {
        buffer[bytes_read] = '\0';
        to_uppercase(buffer);
        printf("Получено и преобразовано: %s\n", buffer);
    }

    if (bytes_read == -1) {
        perror("Ошибка: не удалось прочитать данные");
    }
}

// Функция для закрытия сокетов и удаления файла сокета
void cleanup_socket(int server_fd, const char *path) {
    close(server_fd);
    unlink(path);
}

int main() {
    int server_fd = initialize_socket(SOCKET_PATH);

    printf("Сервер слушает соединения на %s\n", SOCKET_PATH);

    int client_fd = accept(server_fd, NULL, NULL);
    if (client_fd == -1) {
        perror("Ошибка: не удалось принять соединение");
        cleanup_socket(server_fd, SOCKET_PATH);
        exit(EXIT_FAILURE);
    }

    handle_connection(client_fd);

    close(client_fd);
    cleanup_socket(server_fd, SOCKET_PATH);

    return 0;
}

