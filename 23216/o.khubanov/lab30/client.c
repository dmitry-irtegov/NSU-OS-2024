#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCKET_PATH "unix_socket"
#define CHUNK_SIZE 5 // Размер чанка

// Функция для инициализации и подключения клиента
int connect_to_server(const char *path, int *client_fd) {
    struct sockaddr_un server_addr;

    *client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (*client_fd == -1) return -1;

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, path, sizeof(server_addr.sun_path) - 1);

    if (connect(*client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        return -2;
    }

    return 0;
}

int main() {
    int client_fd, result;

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

    char input_buffer[1024]; // Буфер для длинного ввода
    char chunk[CHUNK_SIZE];  // Чанк для отправки

    while (fgets(input_buffer, sizeof(input_buffer), stdin)) {
        if (strncmp(input_buffer, "exit", 4) == 0) {
            break; // Завершаем программу, если введена команда выхода
        }

        size_t input_length = strlen(input_buffer);
        size_t bytes_sent = 0;

        // Отправляем данные чанками
        while (bytes_sent < input_length) {
            size_t chunk_size = (input_length - bytes_sent < CHUNK_SIZE) 
                                ? input_length - bytes_sent 
                                : CHUNK_SIZE;
            memcpy(chunk, input_buffer + bytes_sent, chunk_size);

            if (write(client_fd, chunk, chunk_size) == -1) {
                perror("Ошибка: не удалось отправить данные");
                break;
            }
            bytes_sent += chunk_size;
        }
    }

    close(client_fd);
    printf("Клиент завершил работу.\n");

    return 0;
}

