#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_NAME "sckt"
#define BUFFER_SIZE 1024

int main() {
    int client_socket;
    struct sockaddr_un server_addr;
    pid_t pid;

    client_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Socket creation failed");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_NAME, sizeof(server_addr.sun_path) - 1);

    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Connection to server failed");
        close(client_socket);
        exit(2);
    }

    pid = getpid();

    if (write(client_socket, &pid, sizeof(pid)) == -1) {
        perror("Write PID to server failed");
        close(client_socket);
        exit(3);
    }

    const char *message = "Hello, Server!\n";
    if (write(client_socket, message, strlen(message)) == -1) {
        perror("Write to server failed");
        close(client_socket);
        exit(4);
    }

    printf("Message sent to server (PID: %d)\n", pid);

    close(client_socket);
    return 0;
}