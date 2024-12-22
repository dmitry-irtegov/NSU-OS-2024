#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <poll.h>

#define BUFET_SIZE 1024
#define MAX_CLIENT_COUNT 4

char* socket_path = "./socket_path";

void fdFunc(struct pollfd* fds, int clientCount, int len, int fd) {
    fds[clientCount].fd = fd;
    fds[clientCount].events = POLLIN;
}

void closeFunc(struct pollfd* fds, int clientCount, int len, int index) {
    close(fds[index].fd);

    fds[index] = fds[clientCount];
    memset(fds + clientCount, 0, sizeof(fds[0]));
}

void SIGINTer(int sigNum) {
    printf("Ctrl+C\n");
    unlink(socket_path);
    exit(EXIT_SUCCESS);
}

int main() {
    int server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_socket == -1){
        perror("Error in server socket");
        return 1;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);

    if (bind(server_socket, (struct sockaddr*) &addr, sizeof(addr)) == -1) {
        perror("Error in bind");
        close(server_socket);
        exit(EXIT_FAILURE);
    }
    
    if (signal(SIGINT, SIGINTer) == SIG_ERR) {
        perror("Error whiiile set SIGINT handler");
        close(server_socket);
        unlink(socket_path);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, MAX_CLIENT_COUNT) == -1) {
        perror("Error in listen");
        close(server_socket);
        unlink(socket_path);
        exit(EXIT_FAILURE);
    }

    struct pollfd fds[1 + MAX_CLIENT_COUNT];
    memset(&fds, 0, sizeof(fds));
    struct pollfd *clientFds = fds + 1;
    int clientCount = 0;

    fds[0].fd = server_socket;
    fds[0].events = POLLIN | POLLNVAL | POLLHUP | POLLERR;

    int cnt = 0;
    while (1) {
        int pollCount = poll(fds, 1 + clientCount, 0);
        if (pollCount == -1) {
            perror("Error in poll");
            break;
        }
        
        if (pollCount == 0) {
            continue;
        }

        if (fds[0].revents & (POLLNVAL | POLLHUP | POLLERR)) {
            break;
        }

        if ((clientCount < MAX_CLIENT_COUNT) && (fds[0].revents & POLLIN)) {
            int acceptFD = accept(fds[0].fd, NULL, NULL);
            if (acceptFD == -1) {
                perror("Error in accept");
            }
            else {
                fdFunc(clientFds, clientCount, MAX_CLIENT_COUNT, acceptFD);
                clientCount++;
            }
        }
        
        for (int iter = 0; iter < MAX_CLIENT_COUNT; iter++) {
            if ((clientFds[iter].revents & POLLIN) == 0) {
                continue;
            }

            char bufet[BUFET_SIZE] = "";
            ssize_t cnt_bytes = read(clientFds[iter].fd, bufet, BUFET_SIZE);
            for (int j = 0; j < cnt_bytes; j++) {
                bufet[j] = toupper(bufet[j]);
            }

            if (cnt_bytes <= 0) {
                clientCount--;
                closeFunc(clientFds, clientCount, MAX_CLIENT_COUNT, iter);
                if (cnt_bytes == -1) {
                    perror("Error in read");
                }
            }
            else {
                write(STDOUT_FILENO, bufet, cnt_bytes);
            }
        }
    }
    
    if ((fds[0].revents & POLLNVAL) == 0) {
        close(fds[0].fd);
    }

    for (int i = 0; i < MAX_CLIENT_COUNT; i++) {
        if (fds[i].fd != 0) {
            close(fds[i].fd);
        }
    }

    unlink(socket_path);
    return 0;
}
