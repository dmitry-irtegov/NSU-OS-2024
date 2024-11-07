#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>

#define SIZE 1024

int main() {
    struct sockaddr_un addr;
    char *socket_path = "socket";

    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd == -1) {
        perror("socket error");
        exit(EXIT_FAILURE);
    }
    
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);

    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("connect error");
        close(fd);
        exit(EXIT_FAILURE);
    }
    
    char buf[SIZE];
    ssize_t bytes_read = 0;
    while ((bytes_read = read(STDIN_FILENO, buf, SIZE)) > 0) {
        if (write(fd, buf, bytes_read) == -1) {
            perror("write error");
            close(fd);
            exit(EXIT_FAILURE);
        }   
    }

    if (bytes_read == -1) {
        perror("read error");
        close(fd);
        exit(EXIT_FAILURE);
    }

    close(fd);
    exit(EXIT_SUCCESS);
}
