#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int main() {

    int client_d = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_d == -1) { // если не удалось создать дескриптор для сокета
        perror("socket error");
        exit(1);
    }

    struct sockaddr_un addr; // структура для локального файла. для ipv4 - sockaddr_in, для ipv6 - sockaddr_in6
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    char* socket_path = "./socket";
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path));

    if ((connect(client_d, (struct sockaddr *)&addr, sizeof(addr))) == -1) {
        perror("connect error");
        exit(1);
    }

    printf("Соединение с сервером установлено\n");
    printf("Введите сообщение (Ctrl+D чтобы выйти):\n");
    char text[BUFSIZ];
    char response[BUFSIZ];

    while (fgets(text, BUFSIZ, stdin) != NULL) {
        if (write(client_d, text, strlen(text)) == -1) {
            perror("write error");
            break;
        }

        ssize_t bytes_read = read(client_d, response, sizeof(response) - 1);
        if (bytes_read > 0) {
            response[bytes_read] = '\0';
            printf("Получено с сервера: %s\n", response);
        } else if (bytes_read == 0) {
            printf("Сервер закрыл соединение.\n");
        } else {
            perror("read error");
        }
    }
    
    close(client_d);
    exit(0);
}