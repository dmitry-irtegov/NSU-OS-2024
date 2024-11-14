#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <ctype.h>
#define BUF_SIZE 500

int main(int argc, char *argv[]) {
    int sfd;
    char buf[BUF_SIZE];
    ssize_t nread;
    socklen_t peer_addrlen;
    struct sockaddr_un addr;
    struct sockaddr_un peer_addr;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s socket_path\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    sfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sfd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, argv[1], sizeof(addr.sun_path) - 1);

    if (bind(sfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) == -1) {
        perror("bind");
        close(sfd);
        exit(EXIT_FAILURE);
    }

    for (;;) {
        peer_addrlen = sizeof(peer_addr);
        nread = recvfrom(sfd, buf, BUF_SIZE, 0,
                         (struct sockaddr *)&peer_addr, &peer_addrlen);
        if (nread == -1) {
            perror("recvfrom");
            continue;
        }

        if (strcmp(buf, "end") == 0) {
            printf("end of server\n");
            close(sfd);
            unlink(argv[1]);
            exit(EXIT_SUCCESS);
        }

        for (size_t i = 0; i < strlen(buf); i++) {
            buf[i] = toupper(buf[i]);
        }
        if (strlen(buf) < 500) {
            buf[strlen(buf)] = '\n';
        }

        if (write(fileno(stdin), buf, strlen(buf)) == -1) {
            perror("error in write");
            close(sfd);
            unlink(argv[1]); 
            exit(EXIT_FAILURE);
        }
    }

    close(sfd);
    unlink(argv[1]);
    return 0;
}
