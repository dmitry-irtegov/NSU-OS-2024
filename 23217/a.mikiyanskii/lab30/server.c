#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/unix_socket_example"
#define BUFFER_SIZE 256

void to_uppercase(char *str) {
    for (int i = 0; str[i]; i++) {
        str[i] = toupper(str[i]);
    }
}

int main() {
    int server_sock, client_sock;
    struct sockaddr_un server_addr;
    char buffer[BUFFER_SIZE];

    if ((server_sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(struct sockaddr_un));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    unlink(SOCKET_PATH);

    if (bind(server_sock, (struct sockaddr *) &server_addr, sizeof(struct sockaddr_un)) == -1) {
        perror("bind");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    if (listen(server_sock, 5) == -1) {
        perror("listen");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on %s\n", SOCKET_PATH);

    client_sock = accept(server_sock, NULL, NULL);
    if (client_sock == -1) {
        perror("accept");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    ssize_t num_read;
    num_read = read(client_sock, buffer, BUFFER_SIZE - 1);
    while (num_read > 0) {
        buffer[num_read] = '\0';
        to_uppercase(buffer);
        printf("Received and converted: %s\n", buffer);
    }

    if (num_read == -1) {
        perror("read");
    }

    close(client_sock);
    close(server_sock);
    unlink(SOCKET_PATH);

    return 0;
}
