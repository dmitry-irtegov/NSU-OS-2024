#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCKET_NAME "/tmp/unix_domain_socket"

int main(int argc, char *argv[]) {
    struct sockaddr_un addr;
    int data_socket;

    data_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (data_socket == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(addr));

    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_NAME, sizeof(addr.sun_path) - 1);

    if (connect(data_socket, (const struct sockaddr *) &addr, sizeof(addr)) == -1) {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    char str[] = "hElLo FrOm AnOtHeR PrOcEsS\n";
    write(data_socket, str, strlen(str));

    close(data_socket);

    exit(EXIT_SUCCESS);
}
