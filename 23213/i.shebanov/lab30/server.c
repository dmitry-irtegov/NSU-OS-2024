// server.c
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <ctype.h>
#include <errno.h>

#define BUF_SIZE 1024

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

    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("Binding error");
        close(fd);
        exit(EXIT_FAILURE);
    }

    if (listen(fd, 5) == -1) {
        perror("Listen error");
        unlink(socket_path);
        close(fd);
        exit(EXIT_FAILURE);
    }

    int cl;
    char buf[BUF_SIZE];
    while (1) {
        cl = accept(fd, NULL, NULL);
        
        if(cl > 0){
            unlink(socket_path);
        }else {
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

    if(cl == -1){
        unlink(socket_path);
    }

    close(fd);
    return 0;
}
