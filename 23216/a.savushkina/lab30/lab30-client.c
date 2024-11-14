#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#define BUF_SIZE 500

int main(int argc, char *argv[]) {
    int sfd;
    size_t len;
    struct sockaddr_un addr;

    if (argc < 3) {
        fprintf(stderr, "Usage: %s socket_path msg...\n", argv[0]);
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

    if (connect(sfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) == -1) {
        perror("connect");
        close(sfd);
        exit(EXIT_FAILURE);
    }

    for (size_t j = 2; j < (size_t)argc; j++) {
        len = strlen(argv[j]) + 1;

        if (len > BUF_SIZE) {
            fprintf(stderr,
                    "Ignoring long message in argument %zu\n", j);
            continue;
        }

        if (write(sfd, argv[j], len) != (ssize_t)len) {
            fprintf(stderr, "partial/failed write\n");
            close(sfd);
            exit(EXIT_FAILURE);
        }
    }

    if (write(sfd, "end", 4) != 4) {
        fprintf(stderr, "partial/failed write\n");
        close(sfd);
        exit(EXIT_FAILURE);
    }

    close(sfd);
    exit(EXIT_SUCCESS);
}
