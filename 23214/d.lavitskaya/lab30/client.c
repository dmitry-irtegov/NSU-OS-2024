#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define BUFFSIZE 1024

int main() {
    char* socketPath = "./socket_file";

    int clientSocket_fd;
    clientSocket_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (clientSocket_fd == -1) {
	perror("Socket creation failed");
	exit(1);
    }

    struct sockaddr_un socket_addr;
    memset(&socket_addr, 0, sizeof(socket_addr));
    socket_addr.sun_family = AF_UNIX;
    strncpy(socket_addr.sun_path, socketPath, sizeof(socket_addr.sun_path) - 1);

    if (connect(clientSocket_fd, (struct sockaddr*)&socket_addr, sizeof(socket_addr)) == -1) {
 	close(clientSocket_fd);
	perror("Connect failed");
	exit(1);
    }

    printf("Connected to server socket\n");

    ssize_t bytes_read;
    char buffer[BUFFSIZE];
    int fd = fileno(stdin);
    while ((bytes_read = read(fd, buffer, BUFFSIZE)) > 0) {
        if (write(clientSocket_fd, buffer, bytes_read) == -1) {
            close(clientSocket_fd);
            perror("Write failed");
            exit(1);
        }
    }

    if (bytes_read == -1) {
        close(clientSocket_fd);
        perror("Read failed");
        exit(1);
    }

    close(clientSocket_fd);
    exit(0);
}
