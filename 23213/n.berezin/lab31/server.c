#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/un.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

#define QUEUE_SIZE 10
const char *sock_path = "./sock";
int running = 1;

void handle_sigint(int sig) {
    running = 0;
}

int main() {
    signal(SIGINT, handle_sigint);

    int sockfd;
    if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket error");
        return 1;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, sock_path, sizeof(addr.sun_path) - 1);
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("bind error");
        return 1;
    }

    if (listen(sockfd, QUEUE_SIZE) == -1) {
        perror("listen error");
        unlink(sock_path);
        return 1;
    }

    fd_set read_set;
    FD_ZERO(&read_set);
    FD_SET(sockfd, &read_set);
    int maxfd = sockfd;

    while (running) {
        fd_set ready_set = read_set;
        if (select(maxfd + 1, &ready_set, NULL, NULL, NULL) == -1) {
            if (errno == EINTR) {
                continue;
            }
            perror("select error");
            break;
        }

        for (int fd = 0; fd <= maxfd; fd++) {
            if (FD_ISSET(fd, &ready_set)) {
                if (fd == sockfd) {
                    int connfd = accept(sockfd, NULL, NULL);
                    if (connfd == -1) {
                        perror("accept error");
                        continue;
                    }
                    FD_SET(connfd, &read_set);
                    if (connfd > maxfd) {
                        maxfd = connfd;
                    }
                } else {
                    char buf[1024];
                    ssize_t n = read(fd, buf, sizeof(buf));
                    if (n > 0) {
                        for (int i = 0; i < n; i++) {
                            buf[i] = toupper(buf[i]);
                        }
                        write(STDOUT_FILENO, buf, n);
                    } else if (n == 0) {
                        close(fd);
                        FD_CLR(fd, &read_set);
                    } else {
                        perror("client read error");
                        close(fd);
                        FD_CLR(fd, &read_set);
                    }
                }
            }
        }
    }

    unlink(sock_path);

    return 0;
}
