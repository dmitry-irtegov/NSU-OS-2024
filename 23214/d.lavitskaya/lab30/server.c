#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <ctype.h>
#include <signal.h>

#define BUFFER_SIZE 1024
char* socketPath = "./socket_file";
int serverSocket_fd = -1;
int clientSocket_fd = -1;

void handle_sig(int sig) {
    if (clientSocket_fd != -1) {
        close(clientSocket_fd); 
    }
    if (serverSocket_fd != -1) {
        unlink(socketPath); 
        close(serverSocket_fd); 
    }
    exit(0);
}


int main() {

    signal(SIGINT, handle_sig);
    signal(SIGHUP, handle_sig);

    serverSocket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (serverSocket_fd == -1) {
	perror("Socket creation failed");
	exit(1);
    }

    struct sockaddr_un socket_addr;
    memset(&socket_addr, 0, sizeof(socket_addr));
    socket_addr.sun_family = AF_UNIX;
    strncpy(socket_addr.sun_path, socketPath, sizeof(socket_addr.sun_path) - 1);

    if (bind(serverSocket_fd, (struct sockaddr*) &socket_addr, sizeof(socket_addr)) == -1) {
        close(serverSocket_fd);
	perror("Bind failed");
	exit(1);
    }

    if (listen(serverSocket_fd, 1) == -1) {
        close(serverSocket_fd);
        unlink(socketPath);
        perror("Listen failed");
        exit(1);
    }

    clientSocket_fd  = accept(serverSocket_fd, NULL, NULL);

    if (clientSocket_fd == -1) {
        close(serverSocket_fd);
        unlink(socketPath);
        perror("Accept failed");
        exit(1);
    }

    printf("Accepted client\n");

    ssize_t bytes_read;
    char buffer[BUFFER_SIZE];
    while((bytes_read = read(clientSocket_fd, buffer, BUFFER_SIZE)) > 0) {
        for(int i = 0; i < bytes_read; i++) {
            buffer[i] = toupper((unsigned char)buffer[i]);
        }
        printf("%.*s", (int)bytes_read, buffer);
    }

    if (bytes_read == -1) {
        close(clientSocket_fd);
        close(serverSocket_fd);
        unlink(socketPath);
        perror("Read failed");
        exit(1);
    }

    close(clientSocket_fd);
    close(serverSocket_fd);
    unlink(socketPath);
    exit(0);
}

