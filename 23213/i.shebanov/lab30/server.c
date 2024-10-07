// server.c
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <ctype.h>
#include <errno.h>

int main() {
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
    unlink(socket_path);

    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("Binding error");
        close(fd);
        exit(EXIT_FAILURE);
    }

    if (listen(fd, 5) == -1) {
        perror("Listen error");
        close(fd);
        exit(EXIT_FAILURE);
    }

    int cl;
    char buf[100];
    while (1) {
        if ((cl = accept(fd, NULL, NULL)) == -1) {
            perror("Accept error");
            continue;
        }

        ssize_t rc;
        while ((rc = read(cl, buf, sizeof(buf) - 1)) > 0) {
            buf[rc] = '\0';
            for (int i = 0; i < rc; i++) {
                putchar(toupper((unsigned char)buf[i]));
            }
        }

        if (rc == -1) {
            perror("Read error");
        }
        close(cl);
        break;
    }

    close(fd);
    return 0;
}