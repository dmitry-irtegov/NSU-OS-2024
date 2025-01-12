#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCKET_PATH "unix_socket"
#define BUFFER_SIZE 1024

void to_uppercase(char *str) {
    for (int i = 0; str[i]; i++) {
        str[i] = toupper((unsigned char)str[i]);
    }
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_un server_addr;
    char buffer[BUFFER_SIZE];

    // Создание сокета
    if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Настройка адреса сокета
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    // Удаление старого сокета, если он существует
    unlink(SOCKET_PATH);

    // Привязка сокета
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Прослушивание соединений
    if (listen(server_fd, 5) == -1) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Сервер ожидает соединения...\n");

    // Ожидание клиента
    if ((client_fd = accept(server_fd, NULL, NULL)) == -1) {
        perror("accept");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Клиент подключен.\n");

    // Получение данных от клиента
    ssize_t bytes_read;
    while ((bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1)) > 0) {
        buffer[bytes_read] = '\0';
        to_uppercase(buffer); 
        printf("Получено и преобразовано: %s\n", buffer);
    }

    if (bytes_read == -1) {
        perror("read");
    }

    printf("Клиент отключился.\n");

    // Закрытие соединений
    close(client_fd);
    close(server_fd);

    // Удаление файла сокета
    unlink(SOCKET_PATH);

    return 0;
}
