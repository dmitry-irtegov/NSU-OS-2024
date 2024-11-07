#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

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

    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind error");
        close(fd);
        exit(EXIT_FAILURE);
    }
    
    if (listen(fd, 1) == -1) {
        perror("listen error");
        close(fd);
        unlink(socket_path);
        exit(EXIT_FAILURE);
    }

    int accepted = accept(fd, NULL, NULL);
    if (accepted == -1) {
        perror("accept error");
        close(fd);
        unlink(socket_path);
        exit(EXIT_FAILURE);
    }

    char buf[SIZE];
    ssize_t read_bytes = 0;
    while ((read_bytes = read(accepted, buf, SIZE)) > 0) {
        for (int i = 0; i < read_bytes; i++) {
            printf("%c", toupper(buf[i]));
        }
    }

    if (read_bytes == -1) {
        perror("read error");
        close(accepted);
        close(fd);
        unlink(socket_path);
        return 1;
    }

    close(fd);
    unlink(socket_path);
    exit(EXIT_SUCCESS);
}
