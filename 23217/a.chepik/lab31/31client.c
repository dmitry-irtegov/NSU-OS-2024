#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

int main() {
    const char* socket_path = "my_socket";
    int client_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (client_fd == -1) {
        printf("client socket() failed.\n");
        exit(-1);
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));

    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);

    if (connect(client_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        printf("connect() failed.\n");
        close(client_fd);
        exit(-1);
    }

    char buffer[BUFSIZ];
    ssize_t bytes_read = 0, bytes_write = 0;

    while ((bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer))) > 0) {
        bytes_write = write(client_fd, buffer, bytes_read);

        if (bytes_write == -1) {
            printf("write() failed.\n");
            close(client_fd);
            exit(-1);
        }
    }

    if (bytes_read == -1) {
        printf("client read() failed.\n");
        close(client_fd);
        exit(-1);
    }

    close(client_fd);

    exit(0);
}
