#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <ctype.h>
#define BUF_SIZE 30

int main() {
    int sfd;
    char buf[BUF_SIZE];
    char name[13] = "test_af_unix";
    ssize_t nread;
    socklen_t peer_addrlen;
    struct sockaddr_un addr;
    struct sockaddr_un peer_addr;
    
    sfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sfd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, name, sizeof(addr.sun_path) - 1);
    unlink(name); 
    if (bind(sfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) == -1) {
        perror("bind");
        close(sfd);
        exit(EXIT_FAILURE);
    }

    for (;;) {
        memset(&buf, 0, sizeof(buf));
        peer_addrlen = sizeof(peer_addr);
        nread = recvfrom(sfd, buf, BUF_SIZE, 0,
                         (struct sockaddr *)&peer_addr, &peer_addrlen);
        if (nread == -1) {
            perror("recvfrom");
            continue;
        }

        buf[nread] = '\0';
        if (strcmp(buf, "end") == 0) {
            printf("end of server\n");
            close(sfd);
            unlink(name);
            exit(EXIT_SUCCESS);
        }

        for (size_t i = 0; i < strlen(buf); i++) {
            buf[i] = toupper(buf[i]);
        }
        if (strlen(buf) < 29) {
            buf[strlen(buf)] = '\n';
        }

        if (write(fileno(stdin), buf, strlen(buf)) == -1) {
            perror("error in write");
            close(sfd);
            unlink(name); 
            exit(EXIT_FAILURE);
        }
    }

    close(sfd);
    unlink(name);
    return 0;
}
