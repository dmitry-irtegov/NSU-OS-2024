#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#define BATCHSIZE 1024

char* socket_path = "./socket";

int main(int argc, char* argv[]) {
    char buffer[BATCHSIZE] = "";
    int client_socket;
    client_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("socket error");
        return 1;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);

    if (connect(client_socket, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("connect error");
        close(client_socket);
        return 1;
    }

    ssize_t bytes_read;
    ssize_t written_bytes;
    while ((bytes_read = read(STDIN_FILENO, buffer, BATCHSIZE)) > 0) {
        if (write(client_socket, buffer, bytes_read) == -1) {
            perror("write error");
            close(client_socket);
            return 1;
        }
    }
    if (bytes_read == -1) {
        perror("read error");
        close(client_socket);
        return 1;
    }

    close(client_socket);
    return 0;
}
