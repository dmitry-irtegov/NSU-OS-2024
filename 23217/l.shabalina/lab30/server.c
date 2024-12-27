#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <errno.h>
#include <ctype.h>
#include <signal.h>

#define BUFSIZE 1024

int main() {
    struct sockaddr_un addr;
    int server_fd, client_fd;
    const char *socket_path = "socket1";

    if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket creation fail");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind fail");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 1) == -1) {
        perror("listen fail");
        close(server_fd);
        unlink(socket_path);
        exit(EXIT_FAILURE);
    }

    if ((client_fd = accept(server_fd, NULL, NULL)) == -1) {
        perror("accept fail");
        close(server_fd);
        unlink(socket_path);
        exit(EXIT_FAILURE);
    }

    char buf[BUFSIZE];
    ssize_t bytes_read;
    while ((bytes_read = read(client_fd, buf, sizeof(buf))) > 0) {
        for (ssize_t i = 0; i < bytes_read; i++) {
            printf("%c", toupper(buf[i]));
        }
    }

    if (bytes_read == -1) {
        perror("read fail");
    }

    close(client_fd);
    close(server_fd);
    unlink(socket_path);
    return 0;
}
