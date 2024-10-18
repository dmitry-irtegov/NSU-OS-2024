#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#define BATCHSIZE 1024

char* socket_path = "./socket";

int main(int argc, char* argv[]) {
    int server_socket, client_socket;
    char buffer[BATCHSIZE] = "";
    struct sockaddr_un addr;

    server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_socket == -1){
        perror("socket error");
        return 1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);

    if (bind(server_socket, (struct sockaddr*) &addr, sizeof(addr)) == -1) {
        perror("bind error");
        close(server_socket);
        return 1;
    }

    if (listen(server_socket, 1) == -1) {
        perror("listen error");
        close(server_socket);
        unlink(socket_path);
        return 1;
    }
    client_socket = accept(server_socket, NULL, NULL);
    if (client_socket == -1) {
        perror("accept error");
        close(server_socket);
        unlink(socket_path);
        return 1;
    }

    ssize_t read_bytes = 0;
    while ((read_bytes = read(client_socket, buffer, BATCHSIZE)) > 0) {
        for (int i=0; i < read_bytes; i++) {
            printf("%c", toupper(buffer[i]));
        }
    }
    if (read_bytes == -1) {
        perror("read error");
        close(client_socket);
        close(server_socket);
        unlink(socket_path);
        return 1;
    }
    close(client_socket);
    close(server_socket);
    unlink(socket_path);
    return 0;
}
