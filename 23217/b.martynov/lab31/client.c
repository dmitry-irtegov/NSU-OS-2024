#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define BUFET_SIZE 1024

char* socket_path = "./socket_path";

int main() {
    char bufet[BUFET_SIZE] = { 0 };
    int client_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Error in client socket");
        return 1;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);

    if (connect(client_socket, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("Error in connect");
        close(client_socket);
        return 1;
    }

    ssize_t cnt_bytes = 0;
    while ((cnt_bytes = read(STDIN_FILENO, bufet, BUFET_SIZE)) > 0) {
        if (write(client_socket, bufet, cnt_bytes) == -1) {
            perror("Error in writing");
            close(client_socket);
            return 1;
        }
    }
    if (cnt_bytes == -1) {
        perror("Error in reading");
        close(client_socket);
        return 1;
    }

    close(client_socket);
    return 0;
}
