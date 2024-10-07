// client.c
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    const char string[] = "String with different Registers. Very long string which is longer than buffer to demonstrate it is working correctly\n";
    struct sockaddr_un addr;
    int fd;

    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    const char *socket_path = "socket";
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);

    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("Connection error");
        close(fd);
        exit(EXIT_FAILURE);
    }

    int len = strlen(string);
    int offset = 0;
    char buf[100];

    while (offset < len) {
        int chunk_size = (len - offset > sizeof(buf) - 1) ? (sizeof(buf) - 1) : (len - offset);
        strncpy(buf, string + offset, chunk_size);
        buf[chunk_size] = '\0';

        ssize_t rc = write(fd, buf, chunk_size);
        if (rc == -1) {
            perror("Write error");
            close(fd);
            exit(EXIT_FAILURE);
        }
        offset += chunk_size;
    }

    close(fd);
    return 0;
}