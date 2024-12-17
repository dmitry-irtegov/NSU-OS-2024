#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <poll.h>
#include <pwd.h>
#include <errno.h>

#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10

void to_uppercase(char *str) {
    while (*str) {
        *str = toupper(*str);
        str++;
    }
}

int main() {
    int server_sock;
    struct sockaddr_un server_addr;
    char buffer[BUFFER_SIZE];
    char socket_path[BUFFER_SIZE];
    struct passwd *pw;

    // Получаем текущего пользователя
    pw = getpwuid(getuid());
    if (!pw) {
        perror("Failed to get user info");
        exit(EXIT_FAILURE);
    }

    // Формируем имя сокетного файла
    snprintf(socket_path, sizeof(socket_path), "/tmp/%s_%d_socket", pw->pw_name, getuid());

    // Создаем сокет
    if ((server_sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Удаляем старый сокетный файл, если он существует
    unlink(socket_path);

    // Настраиваем адрес сокета
    memset(&server_addr, 0, sizeof(struct sockaddr_un));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, socket_path, sizeof(server_addr.sun_path) - 1);

    // Привязываем сокет
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_un)) == -1) {
        perror("Bind failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    // Начинаем слушать входящие соединения
    if (listen(server_sock, MAX_CLIENTS) == -1) {
        perror("Listen failed");
        close(server_sock);
        unlink(socket_path);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on %s\n", socket_path);

    struct pollfd clients[MAX_CLIENTS + 1]; // +1 для серверного сокета
    clients[0].fd = server_sock;           // Серверный сокет
    clients[0].events = POLLIN;            // Ждем входящих соединений

    for (int i = 1; i <= MAX_CLIENTS; i++) {
        clients[i].fd = -1; // Инициализация
    }

    while (1) {
        int poll_count = poll(clients, MAX_CLIENTS + 1, -1); // Ждем событий
        if (poll_count == -1) {
            perror("Poll failed");
            break;
        }

        // Проверяем серверный сокет на новые соединения
        if (clients[0].revents & POLLIN) {
            int new_client = accept(server_sock, NULL, NULL);
            if (new_client == -1) {
                perror("Accept failed");
            } else {
                printf("New client connected\n");
                // Добавляем клиента в список
                for (int i = 1; i <= MAX_CLIENTS; i++) {
                    if (clients[i].fd == -1) {
                        clients[i].fd = new_client;
                        clients[i].events = POLLIN;
                        break;
                    }
                }
            }
        }

        // Проверяем данные от клиентов
        for (int i = 1; i <= MAX_CLIENTS; i++) {
            if (clients[i].fd != -1 && clients[i].revents & POLLIN) {
                ssize_t num_read = read(clients[i].fd, buffer, BUFFER_SIZE - 1);
                if (num_read > 0) {
                    buffer[num_read] = '\0';
                    to_uppercase(buffer);
                    printf("Received and converted to uppercase: %s\n", buffer);
                } else {
                    // Закрываем соединение, если клиент отключился
                    printf("Client disconnected\n");
                    close(clients[i].fd);
                    clients[i].fd = -1;
                }
            }
        }
    }

    printf("Shutting down server...\n");
    close(server_sock);
    unlink(socket_path);
    return 0;
}
