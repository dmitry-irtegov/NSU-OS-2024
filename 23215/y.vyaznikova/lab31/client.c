#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <time.h>

#define SOCKET_NAME "sckt"
#define BUFFER_SIZE 10
#define BYTE_DELAY_MS 10
#define MAX_LIFETIME_SEC 10

#define CLIENT_FAST 0
#define CLIENT_LONG_LIVED 1
#define CLIENT_SELF_DISCONNECT 2

ssize_t write_n_bytes(int fd, const void *buffer, size_t n) {
    size_t bytes_left = n;
    size_t bytes_written = 0;
    const char *buf = (const char *)buffer;

    while (bytes_left > 0) {
        ssize_t result = write(fd, buf + bytes_written, 1);
        
        if (result < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        
        bytes_written += result;
        bytes_left -= result;
        
        struct timespec ts = {0, 10000000};
        nanosleep(&ts, NULL);
    }
    
    return bytes_written;
}

void msleep(int milliseconds) {
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

int main(void) {
    srand(time(NULL) + getpid());
    
    int client_type = rand() % 3;
    int lifetime = 0;
    
    if (client_type == CLIENT_LONG_LIVED) {
        lifetime = MAX_LIFETIME_SEC;
    } else if (client_type == CLIENT_SELF_DISCONNECT) {
        lifetime = 2 + (rand() % 4);
    }

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
        if (errno == ECONNREFUSED) {
            printf("Connection rejected by server\n");
        } else {
            perror("Connect failed");
        }
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
            ssize_t written = write_n_bytes(client_socket, input_buffer + bytes_sent, 1);
            if (written < 0 || written != 1) {
                perror("Failed to send message chunk");
                goto cleanup;
            }
            
            msleep(BYTE_DELAY_MS);
            bytes_sent += 1;
        }

        if (lifetime > 0) {
            printf("Client will stay connected for %d seconds\n", lifetime);
            sleep(lifetime);
        }
        break;
    }

cleanup:
    close(client_socket);
    return 0;
}