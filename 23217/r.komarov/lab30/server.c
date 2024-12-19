#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <ctype.h>
#define SOCKET_PATH "./socket"

int main() {
    int server_fd;

    if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_un server_addr;
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    unlink(SOCKET_PATH);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 1) == -1) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }
    
    int client_fd;

    if ((client_fd = accept(server_fd, NULL, NULL)) == -1) {
        perror("accept failed");
        exit(EXIT_FAILURE);
    }
    
    printf("Client connected\n");
    
    char buffer[BUFSIZ];
    ssize_t bytes_read;
    
    while ((bytes_read = read(client_fd, buffer, BUFSIZ - 1)) > 0) {
        buffer[bytes_read] = '\0';
        for (int i = 0; buffer[i]; i++) {
            buffer[i] = toupper(buffer[i]);
        }

        if (bytes_read == -1) {
            perror("read failed");
        }
        
        if (write(client_fd, buffer, bytes_read) == -1) {
            perror("write error");
            exit(EXIT_FAILURE);
        }
    }


    close(client_fd);
    close(server_fd);
 
    unlink(SOCKET_PATH);

    return 0;
}

