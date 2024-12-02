#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>

#define SOCKET_NAME "socket"
#define BUFFER_SIZE 10

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

int main() {
    int client_socket;
    struct sockaddr_un server_addr;

    client_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Socket creation failed");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_NAME, sizeof(server_addr.sun_path) - 1);

    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Connect failed");
        exit(2);
    }

    char input_buffer[1024];
    while (fgets(input_buffer, sizeof(input_buffer), stdin) != NULL) {
        uint32_t total_size = strlen(input_buffer);
        
        if (write_n_bytes(client_socket, &total_size, sizeof(total_size)) != sizeof(total_size)) {
            perror("Failed to send message size");
            break;
        }

        size_t bytes_sent = 0;
        while (bytes_sent < total_size) {
            size_t chunk_size = (total_size - bytes_sent > BUFFER_SIZE - 1) ? 
                BUFFER_SIZE - 1 : total_size - bytes_sent;
            
            ssize_t written = write_n_bytes(client_socket, input_buffer + bytes_sent, chunk_size);
            if (written < 0 || (size_t)written != chunk_size) {
                perror("Failed to send message chunk");
                goto cleanup;
            }
            
            bytes_sent += chunk_size;
        }
    }

cleanup:
    close(client_socket);
    return 0;
}