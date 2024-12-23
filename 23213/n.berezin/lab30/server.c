#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

const char *sock_path = "./.sock";

int main() {
    int sockfd;
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, sock_path, sizeof(addr.sun_path) - 1);

    if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        return 1;
    }

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("bind");
        return 1;
    }

    if (listen(sockfd, 5) == -1) {
        perror("listen");
        return 1;
    }

    socklen_t clilen = sizeof(addr);

    int connfd = accept(sockfd, (struct sockaddr *)&addr, &clilen);
    if (connfd == -1) {
        perror("accept");
        return 1;
    }

    char buf[1024];
    ssize_t n;
    while ((n = read(connfd, buf, sizeof(buf))) > 0) {
        for (int i = 0; i < n; i++) {
            buf[i] = toupper(buf[i]);
        }
        write(STDOUT_FILENO, buf, n);
    }
    unlink(sock_path);

    return 0;
}
