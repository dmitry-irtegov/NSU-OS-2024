#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/unix_socketasdasd"
#define BUFFER_SIZE 1024

int main() {
    int client_sock;
    struct sockaddr_un server_addr;
    char buffer[BUFFER_SIZE];

    // Создаем сокет
    if ((client_sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Настраиваем адрес сервера
    memset(&server_addr, 0, sizeof(struct sockaddr_un));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    // Подключаемся к серверу
    if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_un)) == -1) {
        perror("Connect failed");
        close(client_sock);
        exit(EXIT_FAILURE);
    }

    // Передаем текст серверу
    printf("Enter text to send to server (Ctrl+D to finish):\n");
    while (fgets(buffer, BUFFER_SIZE, stdin)) {
        size_t total_written = 0;                     // Сколько байт уже записано
        size_t bytes_to_write = strlen(buffer);       // Сколько байт нужно записать

        while (total_written < bytes_to_write) {
            ssize_t bytes_written = write(client_sock, buffer + total_written, bytes_to_write - total_written);
            if (bytes_written == -1) {
                perror("Write failed");
                close(client_sock);
                exit(EXIT_FAILURE);
            }
            total_written += bytes_written;           // Увеличиваем счетчик записанных байт
        }
    }

    // Закрываем соединение
    close(client_sock);
    return 0;
}