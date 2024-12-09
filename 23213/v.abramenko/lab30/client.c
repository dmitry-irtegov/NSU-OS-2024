#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/un.h>

#define BUF_SIZE 1024
char* socket_path = "socket";

int main(void) {
    struct sockaddr_un addr;
    int fd;

    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket failed");
        exit(-1);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);

    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("connect failed");
        exit(-1);
    }

    char buf[BUF_SIZE];
    ssize_t rc;
    while ((rc = read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
        if (write(fd, buf, rc) == -1) {
            perror("write failed");
            exit(-1);
        }
    }

    if (rc == -1) {
        perror("read failed");
        exit(-1);
    }

    exit(0);
}
