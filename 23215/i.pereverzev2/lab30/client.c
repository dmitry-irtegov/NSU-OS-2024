#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <string.h>
#include <unistd.h>

#define SOCKNAME "socket"
#define BUFLENGTH 8

int main() {
    int sockdes = 0;
    if (sockdes == -1) {
        perror("can't create socket");
        return 1;
    }
    struct sockaddr_un sock;
    sock.sun_family = AF_UNIX;
    strcpy(sock.sun_path, SOCKNAME);
    sockdes = socket(PF_UNIX, SOCK_STREAM, 0);
    if (sockdes == -1) {
        perror("can't create socket");
        return 1;
    }
    if (connect(sockdes, (struct sockaddr*)&sock, sizeof(struct sockaddr_un)) == -1) {
        perror("unable to connect from client");
        return 5;
    }
    char buf[] = "Sample Text That Will Be Written inTo the socket";
    if (write(sockdes, buf, strlen(buf)) == -1) {
        perror("unable to write into the socket from client");
        return 6;
    }
    return 0;
}
