#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <ctype.h>

#define SOCKET_FILE "/tmp/alternative_socket"
#define BUF_SIZE 256

void handleError(const char *msg, int socket_fd) {
    perror(msg);
    if (socket_fd != -1) {
        close(socket_fd);
    }
    unlink(SOCKET_FILE);
    exit(EXIT_FAILURE);
}

int main() {
    int sock_fd = -1, conn_fd = -1;
    struct sockaddr_un sock_addr;
    char data[BUF_SIZE];
    ssize_t received;

    if ((sock_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        handleError("ошибка создания сокета", -1);
    }

    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sun_family = AF_UNIX;
    strncpy(sock_addr.sun_path, SOCKET_FILE, sizeof(sock_addr.sun_path) - 1);

    unlink(SOCKET_FILE);

    if (bind(sock_fd, (struct sockaddr *)&sock_addr, sizeof(sock_addr)) == -1) {
        handleError("ошибка привязки сокета", sock_fd);
    }

    if (listen(sock_fd, 5) == -1) {
        handleError("ошибка прослушивания сокета", sock_fd);
    }

    printf("сервер запущен, ожидание подключения по адресу: %s\n", SOCKET_FILE);

    if ((conn_fd = accept(sock_fd, NULL, NULL)) == -1) {
        handleError("ошибка принятия подключения", sock_fd);
    }

    printf("клиент подключился\n");

    while ((received = read(conn_fd, data, BUF_SIZE)) > 0) {
        for (int i = 0; i < received; i++) {
            data[i] = toupper((unsigned char)data[i]);
        }
        write(STDOUT_FILENO, data, received);
    }

    close(conn_fd);
    close(sock_fd);
    unlink(SOCKET_FILE);

    printf("\nсервер завершил работу.\n");
    return 0;
}