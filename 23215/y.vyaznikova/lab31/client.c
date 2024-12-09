#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <stdint.h>

#define SOCKET_NAME "sckt"
#define BUFFER_SIZE 100

ssize_t write_n_bytes(int fd, const void *buffer, size_t n) {
    size_t bytes_left = n;
    size_t bytes_written = 0;
    const char *buf = (const char *)buffer;

    while (bytes_left > 0) {
        ssize_t result = write(fd, buf + bytes_written, bytes_left);
        
        if (result < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        
        bytes_written += result;
        bytes_left -= result;
    }
    
    return bytes_written;
}

int main(void) {
    int client_socket;
    struct sockaddr_un server_addr;
    char buffer[BUFFER_SIZE];

    client_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Socket creation failed");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_NAME, sizeof(server_addr.sun_path) - 1);

    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        if (errno == ECONNREFUSED) {
            printf("Connection rejected by server\n");
        } else {
            perror("Connect failed");
        }
        close(client_socket);
        exit(2);
    }

    while (fgets(buffer, BUFFER_SIZE, stdin) != NULL) {
        uint32_t msg_size = strlen(buffer);
        
        if (write_n_bytes(client_socket, &msg_size, sizeof(msg_size)) == -1) {
            perror("Failed to send message size");
            break;
        }
        
        if (write_n_bytes(client_socket, buffer, msg_size) == -1) {
            perror("Failed to send message");
            break;
        }
    }

    close(client_socket);
    return 0;
}