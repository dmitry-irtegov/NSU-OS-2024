#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <signal.h>

#define SOCKET_PATH "/tmp/unix_socketasdasd"
#define BUFFER_SIZE 1024

volatile sig_atomic_t stop = 0;

void handle_sigint(int sig) {
    stop = 1;
}

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

    // Устанавливаем обработчик сигнала
    signal(SIGINT, handle_sigint);

    // Принимаем и обрабатываем входящие соединения в цикле
    while (!stop) {
        // Принимаем новое соединение
        if ((client_sock = accept(server_sock, NULL, NULL)) == -1) {
            perror("Accept failed");
            continue;  // Продолжаем принимать новые соединения
        }

        printf("Client connected\n");

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

        printf("Client disconnected\n");

        // Закрываем клиентский сокет
        close(client_sock);
    }

    printf("Shutting down server...\n");

    // Закрываем серверный сокет и удаляем сокетный файл (не достигается из-за бесконечного цикла)
    close(server_sock);
    unlink(SOCKET_PATH);

    return 0;
}
