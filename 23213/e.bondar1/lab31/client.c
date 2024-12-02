#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/un.h>

#define BATCHSIZE 1024
#define MIN(a, b) ((a) < (b) ? (a) : (b))

char* socket_path = "./socket";

int main(void) {
    int sock;
    pid_t pid = getpid();

    if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }
    printf("Client pid: %d created client socket\n", pid);

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);

    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("Failed to connect");
        exit(EXIT_FAILURE);
    }
    printf("Client pid: %d connected to server\n", pid);

    char buffer[BATCHSIZE];
    ssize_t bytes_read;

    while ((bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer))) > 0) {
        ssize_t current_sent = write(sock, buffer, MIN(bytes_read, sizeof(buffer)));
        if (current_sent == -1) {
            perror("Failed to write to server socket");
            exit(EXIT_FAILURE);
        }
    }

    if (bytes_read == -1) {
        perror("Error reading from stdin");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
