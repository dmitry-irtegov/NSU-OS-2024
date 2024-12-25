#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define SOCKET_PATH "/tmp/unix_socket"

void sender(int sock_fd, char *message){
	write(sock_fd, message, strlen(message));
}

int main() {
    int sock_fd;
    struct sockaddr_un server_addr;
    char *message = "Hello, Unix Domain Socket!\n";
    
    sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    sender(sock_fd, message);
    close(sock_fd);
    return 0;
}
