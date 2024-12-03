#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCKET_PATH "unix_socket"
#define CHUNK_SIZE 5 // Размер чанка

void to_uppercase(char *str, size_t len) {
    size_t i;
    for (size_t i = 0; i < len; i++) {
        str[i] = toupper((unsigned char)str[i]);
    }
}

// Функция для инициализации сокета
int initialize_socket(const char *path, int *server_fd) {
    struct sockaddr_un server_addr;

    *server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (*server_fd == -1) return -1;

    unlink(path);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, path, sizeof(server_addr.sun_path) - 1);

    if (bind(*server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        return -2;
    }

    if (listen(*server_fd, 5) == -1) {
        return -3;
    }

    return 0;
}

// Функция для обработки соединения чанками с восстановлением строки
int handle_connection(int client_fd) {
    char chunk[CHUNK_SIZE];
    char buffer[1024]; // Буфер для сборки строки
    size_t buffer_len = 0;
    ssize_t bytes_read;

    while ((bytes_read = read(client_fd, chunk, CHUNK_SIZE)) > 0) {
        // Скопируем чанк в буфер
        for (ssize_t i = 0; i < bytes_read; i++) {
            buffer[buffer_len++] = chunk[i];

            // Если найден конец строки, обрабатываем собранную строку
            if (chunk[i] == '\n' || buffer_len >= sizeof(buffer) - 1) {
                buffer[buffer_len] = '\0'; // Завершаем строку
                to_uppercase(buffer, buffer_len);
                printf("Получено и преобразовано: %s", buffer);
                buffer_len = 0; // Сбрасываем буфер
            }
        }
    }

    if (bytes_read == -1) {
        perror("Ошибка чтения данных");
        return -1;
    }
    return 0;
}

int main() {
    int server_fd, client_fd, result;

    result = initialize_socket(SOCKET_PATH, &server_fd);
    result = initialize_socket(SOCKET_PATH, &server_fd);
    if (result != 0) {
    	switch (result) {
        	case -1:
            		fprintf(stderr, "Ошибка: не удалось создать сокет.\n");
            		break;
        	case -2:
            		fprintf(stderr, "Ошибка: не удалось привязать сокет. Возможно, сервер уже работает.\n");
            		break;
        	case -3:
           		 fprintf(stderr, "Ошибка: не удалось настроить прослушивание соединений.\n");
            		 break;
        	default:
            		 fprintf(stderr, "Неизвестная ошибка: код %d\n", result);
                         break;
    	}
    	exit(EXIT_FAILURE);
    }


    printf("Сервер слушает соединения на %s\n", SOCKET_PATH);

    client_fd = accept(server_fd, NULL, NULL);
    if (client_fd == -1) {
        perror("Ошибка: не удалось принять соединение");
        close(server_fd);
        unlink(SOCKET_PATH);
        exit(EXIT_FAILURE);
    }

    result = handle_connection(client_fd);
    if (result == -1) {
        fprintf(stderr, "Ошибка: не удалось обработать данные от клиента.\n");
    }

    close(client_fd);
    close(server_fd);
    unlink(SOCKET_PATH);

    printf("Сервер завершил работу.\n");

    return 0;
}

