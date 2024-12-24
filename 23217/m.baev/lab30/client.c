#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/baev_unix_socket"
#define CONNECTION_QUEUE 1
#define BUFFER_SIZE 1024

int sfd = -1;

void handle_signal(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        exit(EXIT_SUCCESS);
    }
}

void cleanup() {
    if (sfd >= 0) close(sfd);
}

int main() {

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);


    atexit(cleanup);
    
    sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCKET_PATH);
    

    if (connect(sfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];

    while(1) {
        memset(buffer, 0, BUFFER_SIZE);
        read(0, buffer, BUFFER_SIZE);
        write(sfd, buffer, BUFFER_SIZE);
    }

    close(sfd);

    return 0;
}