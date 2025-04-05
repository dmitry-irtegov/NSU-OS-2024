#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <ctype.h>

const char* socket_path = "my_socket";

void my_unlink() {
    unlink(socket_path);
    printf("Unlink is done.\n");
}

int main() {
    int socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (socket_fd == -1) {
        printf("server socket() failed.\n");
        exit(-1);
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));

    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);

    if (bind(socket_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        printf("bind() failed.\n");
        close(socket_fd);
        exit(-1);
    }

    if (atexit(my_unlink) != 0) {
        printf("atexit() failed.\n");
        close(socket_fd);
        exit(-1);
    }

    if (listen(socket_fd, 1) != 0) {
        printf("listen() failed.\n");
        close(socket_fd);
        exit(-1);
    }

    int client_fd = accept(socket_fd, NULL, NULL);

    if (client_fd == -1) {
        printf("accept() failed.\n");
        close(socket_fd);
        exit(-1);
    }

    char buffer[BUFSIZ];
    ssize_t bytes_read = 0;

    while ((bytes_read = read(client_fd, buffer, sizeof(buffer))) > 0) {
        for (ssize_t i = 0; i < bytes_read; i++) {
            printf("%c", toupper(buffer[i]));
        }
    }

    if (bytes_read == -1) {
        printf("server read() failed.\n");
        close(client_fd);
        close(socket_fd);
        exit(-1);
    }

    close(client_fd);
    close(socket_fd);

    exit(0);
}
