#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "unix_socket"
#define BUFFER_SIZE 1024

void to_uppercase(char *str) {
    while (*str) {
        *str = toupper(*str);
        str++;
    }
}

int main() {
    int server_sock, client_sock;
    struct sockaddr_un server_addr;
    char buffer[BUFFER_SIZE];

    // Создаем сокет
    if ((server_sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Удаляем старый сокетный файл, если он существует
    unlink(SOCKET_PATH);

    // Настраиваем адрес сокета
    memset(&server_addr, 0, sizeof(struct sockaddr_un));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    // Привязываем сокет
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_un)) == -1) {
        perror("Bind failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    // Начинаем слушать входящие соединения
    if (listen(server_sock, 5) == -1) {
        perror("Listen failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on %s\n", SOCKET_PATH);

    // Принимаем входящее соединение
    if ((client_sock = accept(server_sock, NULL, NULL)) == -1) {
        perror("Accept failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    // Читаем данные от клиента
    ssize_t num_read;
    while ((num_read = read(client_sock, buffer, BUFFER_SIZE - 1)) > 0) {
        buffer[num_read] = '\0';  // Завершаем строку
        to_uppercase(buffer);
        printf("Received and converted to uppercase: %s\n", buffer);
    }

    if (num_read == -1) {
        perror("Read failed");
    }

    // Закрываем соединения и удаляем сокетный файл
    close(client_sock);
    close(server_sock);
    unlink(SOCKET_PATH);

    return 0;
}
