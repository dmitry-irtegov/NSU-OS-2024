#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <strings.h>
#include <ctype.h>

#define SOCKNAME "socket"
#define BUFLENGTH 8

int main() {
    int sockdes = 0;
    struct sockaddr_un sock;
    sock.sun_family = AF_UNIX;
    strncpy(sock.sun_path, SOCKNAME, sizeof(sock.sun_path - 1));
    sockdes = socket(PF_UNIX, SOCK_STREAM, 0);
    if (sockdes == -1) {
        perror("unable to create socket");
        return 1;
    }
    if (bind(sockdes, (struct sockaddr*)&sock, sizeof(struct sockaddr_un)) == -1) {
        perror("unable to bind");
        unlink(SOCKNAME);
        return 2;
    }
    if (listen(sockdes, 1) == -1) {
        perror("unable to listen");
        unlink(SOCKNAME);
        return 3;
    }
    int acceptedfd = accept(sockdes, NULL, NULL);
    if (acceptedfd == -1) {
        perror("unable to accept");
        unlink(SOCKNAME);
        return 4;
    }
    unlink(SOCKNAME);
    char buf[BUFLENGTH];
    ssize_t readed = 0;
    while ((readed = read(acceptedfd, buf, BUFLENGTH)) > 0) {
        for (int i = 0; i < readed; i++) {
            buf[i] = toupper(buf[i]);
            printf("%c", buf[i]);
        }
    }
    printf("\n");
    return 0;
}
