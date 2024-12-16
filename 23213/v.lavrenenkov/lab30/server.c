#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#define BACKLOG 0


char* path = "./soc";

int main(int argc, char **argv) {
    int fd;
    if ( (fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket error");
        return 1;
    }
    struct sockaddr_un adr;
    memset(&adr, 0, sizeof(adr));
    adr.sun_family = AF_UNIX;
    strncpy(adr.sun_path, path, sizeof(adr.sun_path) - 1);
    if (bind(fd, (struct sockaddr*)&adr, sizeof(adr)) == -1) {
        perror("Couldn`t bind");
        return 1;
    }
    if (listen(fd, BACKLOG) == -1) {
        perror("Couldn`t listen");
        unlink(path);
        return 1;
    }
    int clientFd;
    if ((clientFd = accept(fd, NULL, NULL)) == -1) {
        perror("Couldn`t accept");
        unlink(path);
        return 1;
    }
    char buf[BUFSIZ];
    ssize_t readed;
    while ((readed = read(clientFd, buf, sizeof(buf))) > 0) {
        for(int i = 0; i < readed; i++) {
            buf[i] = toupper(buf[i]);
        }
        printf("%.*s", readed, buf);
    }
    unlink(path);
    return 0;
}
