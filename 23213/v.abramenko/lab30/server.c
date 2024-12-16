#include <sys/socket.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <ctype.h>
#include <errno.h>

#define BUF_SIZE 1024
char* socket_path = "./socket";

int main() {
    struct sockaddr_un addr;
    int fd;

    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket failed");
        exit(-1);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);

    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind failed");
        exit(-1);
    }

    chmod(socket_path, 0666);

    if (listen(fd, 1) == -1) {
        perror("listen failed");
        unlink(socket_path);
        exit(-1);
    }

    int cl;
    char buf[BUF_SIZE];
    ssize_t rc;
    while (1) {
        if ((cl = accept(fd, NULL, NULL)) == -1) {
            perror("accept failed");
            continue;
        }
        unlink(socket_path);
        while ((rc = read(cl, buf, sizeof(buf))) > 0) {
            for (int i = 0; i < rc; i++) {
                putchar(toupper((unsigned char)buf[i]));
            }
        }
        if (rc == -1) {
            perror("read failed");
            exit(-1);
        }
        break;
    }
    exit(0);
}
