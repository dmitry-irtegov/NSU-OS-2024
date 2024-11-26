#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define BUF_SIZE 30

void send_in_chunks(int sfd, const char *msg, size_t len) {
    size_t offset = 0;

    while (offset < len) {
        size_t chunk_size = (len - offset) > BUF_SIZE ? BUF_SIZE : (len - offset);
        if (write(sfd, msg + offset, chunk_size) != (ssize_t)chunk_size) {
            fprintf(stderr, "partial/failed write\n");
            close(sfd);
            exit(EXIT_FAILURE);
        }
        offset += chunk_size;
    }
}

int main(int argc, char *argv[]) {
    int sfd;
    char name[13] = "test_af_unix";
    char end[4] = "end";
    struct sockaddr_un addr;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s msg...\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    sfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sfd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, name, sizeof(addr.sun_path) - 1);

    if (connect(sfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) == -1) {
        perror("connect");
        close(sfd);
        exit(EXIT_FAILURE);
    }

    for (size_t j = 1; j < (size_t)argc; j++) {
        size_t len = strlen(argv[j]) + 1;

        send_in_chunks(sfd, argv[j], len);

    }

    if (write(sfd, end, strlen(end)) != (ssize_t)strlen(end)) {
        fprintf(stderr, "partial/failed write\n");
        close(sfd);
        exit(EXIT_FAILURE);
    }

    close(sfd);
    exit(EXIT_SUCCESS);
}
