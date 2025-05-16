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
    int server, client;
    char buffer[BATCHSIZE] = "";
    struct sockaddr_un addr;

    server = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server == -1){
        perror("socket error");
        return 1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);

    if (bind(server, (struct sockaddr*) &addr, sizeof(addr)) == -1) {
        perror("bind error");
        close(server);
        return 1;
    }

    if (listen(server, 1) == -1) {
        perror("listen error");
        close(server);
        unlink(socket_path);
        return 1;
    }
    client = accept(server, NULL, NULL);
    if (client == -1) {
        perror("accept error");
        close(server);
        unlink(socket_path);
        return 1;
    }

    ssize_t read_bytes = 0;
    while ((read_bytes = read(client, buffer, BATCHSIZE)) > 0) {
        for (int i=0; i < read_bytes; i++) {
            printf("%c", toupper(buffer[i]));
        }
    }
    if (read_bytes == -1) {
        perror("read error");
        close(client);
        close(server);
        unlink(socket_path);
        return 1;
    }
    close(client);
    close(server);
    unlink(socket_path);
    return 0;
}
