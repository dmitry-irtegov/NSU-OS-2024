#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>

#define SOCKET_PATH "/tmp/baev_unix_socket"
#define CONNECTION_QUEUE 1
#define BUFFER_SIZE 1024

int sfd = -1;
int cfd = -1;

void handle_signal(int sig) {
    exit(EXIT_SUCCESS);
}

void cleanup() {
    unlink(SOCKET_PATH);
    if (sfd >= 0) close(sfd);
    if (cfd >= 0) close(cfd);
}

int main() {

    atexit(cleanup);

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);


    if (bind(sfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }


    if (listen(sfd, CONNECTION_QUEUE) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }


    cfd = accept(sfd, NULL, NULL);
    if (cfd < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        ssize_t bytes_read = read(cfd, buffer, BUFFER_SIZE);

        if (bytes_read > 0) {
            for (ssize_t i = 0; i < bytes_read; i++) {
                putchar(toupper(buffer[i]));
            }
        } else if (bytes_read == 0) {
            printf("Client disconnected\n");
            break;
        } else {
            perror("Read failed");
            break;
        }
    }

    return 0;
}