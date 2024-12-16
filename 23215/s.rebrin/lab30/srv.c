#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <ctype.h>

char *socket_path = "./socket";
//char* socket_path = "\0hidden";

int main(int argc, char* argv[]) {
    struct sockaddr_un addr;
    char buf[100];
    int fd, cl, rc;

    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket error");
        exit(-1);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    if (*socket_path == '\0') {
        *addr.sun_path = '\0';
        strncpy(addr.sun_path + 1, socket_path + 1, sizeof(addr.sun_path) - 2);
    }
    else {
        strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);
        unlink(socket_path);
    }

    unlink(socket_path);
    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind error");
        unlink(socket_path);
        exit(-1);
    }

    if (listen(fd, 5) == -1) {
        perror("listen error");
        exit(-1);
    }

        if ((cl = accept(fd, NULL, NULL)) == -1) {
            perror("accept error");
            unlink(socket_path);
            continue;
        }

        while ((rc = read(cl, buf, sizeof(buf))) > 0) {
            for (int i = 0; i < rc; i++) {
                buf[i] = toupper(buf[i]);
            }
            write(1, buf, rc);
        }
        if (rc == -1) {
            perror("read");
            unlink(socket_path);
            exit(-1);
        }
        else if (rc == 0) {
            printf("EOF\n");
            close(cl);
            unlink(socket_path);
            exit(0);
        


    return 0;
}