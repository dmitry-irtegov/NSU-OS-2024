#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>

#define SOCKET_FILE "/tmp/alternative_socket"
#define BUF_SIZE 256

void onBrokenPipe(int signal) {
    (void)signal;
    fprintf(stderr, "соединение с сервером потеряно\n");
    exit(EXIT_FAILURE);
}

int connectToServer(const char *socketPath) {
    int socket_fd;
    struct sockaddr_un server_address;

    socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        perror("не удалось создать сокет");
        exit(EXIT_FAILURE);
    }

    memset(&server_address, 0, sizeof(server_address));
    server_address.sun_family = AF_UNIX;
    strncpy(server_address.sun_path, socketPath, sizeof(server_address.sun_path) - 1);

    if (connect(socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("ошибка подключения к серверу");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    return socket_fd;
}

int main() {
    int client_socket;
    char buffer[BUF_SIZE];
    ssize_t bytesRead;

    signal(SIGPIPE, onBrokenPipe);

    client_socket = connectToServer(SOCKET_FILE);
    printf("подключились к серверу, введите сообщение (CTRL+D для выхода):\n");

    while ((bytesRead = read(STDIN_FILENO, buffer, BUF_SIZE)) > 0) {
        if (write(client_socket, buffer, bytesRead) == -1) {
            perror("ошибка при отправке данных");
            close(client_socket);
            exit(EXIT_FAILURE);
        }
    }

    close(client_socket);
    printf("соединение закрыто\n");

    return 0;
}