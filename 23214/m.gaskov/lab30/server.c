#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCKET_PATH "unix_socket"
#define BUFFER_SIZE 1024

void to_uppercase(char *str) {
    int i;
    for (i = 0; str[i]; i++) {
        str[i] = toupper((unsigned char)str[i]);
    }
}

int main() {
    int listen_fd, connected_fd;
    struct sockaddr_un addr;

    if ((listen_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    if (strlen(SOCKET_PATH) >= sizeof(addr.sun_path)) {
        fprintf(stderr, "Socket path is very long.");
        exit(EXIT_FAILURE);
    }
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path));

    if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("bind");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(listen_fd, 1) == -1) {
        perror("listen");
        close(listen_fd);
        unlink(SOCKET_PATH);
        exit(EXIT_FAILURE);
    }

    if ((connected_fd = accept(listen_fd, NULL, NULL)) == -1) {
        perror("accept");
        close(listen_fd);
        unlink(SOCKET_PATH);
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    while ((bytes_read = read(connected_fd, buffer, BUFFER_SIZE - 1)) > 0) {
        buffer[bytes_read] = '\0';
        to_uppercase(buffer);
        printf("%s", buffer);
        fflush(stdout);
    }

    close(connected_fd);
    close(listen_fd);
    unlink(SOCKET_PATH);
    if (bytes_read == -1) {
        perror("read");
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}
