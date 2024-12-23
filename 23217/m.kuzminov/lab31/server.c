#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <poll.h>
#include <ctype.h>
#include <errno.h>

int main() {
    int fd;
    char* socket_name = "./unix_socket";

    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket creation failed");
        exit(1);
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, socket_name);


    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind failed");
        exit(1);
    }

    if (listen(fd, 2) == -1) {
        perror("listen failed");
        exit(1);
    }

    int nfdsmax = 3;
    struct pollfd fds[nfdsmax];
    int nfds = 1;
    memset(fds, 0, sizeof(fds));
    fds[0].fd = fd; 
    fds[0].events = POLLIN;
    
    printf("Wait events has begun\n");

    while (1) {
        int poll_count = poll(fds, nfds, -1);

        if (poll_count == -1) {
            perror("poll");
            break;
        }

        for (int i = 0; i < nfds; i++) {
            if (fds[i].revents & POLLIN) {
                if (fds[i].fd == fd) {
                    if(nfds < nfdsmax) {
                        int acfd;
                        if((acfd = accept(fd, NULL, NULL)) == -1) {
                            perror("accept");
                            continue;
                        }
                        fds[nfds].fd = acfd;
                        fds[nfds].events = POLLIN;
                        nfds++;
                    } else {
                        int acfd;
                        if((acfd = accept(fd, NULL, NULL)) == -1) {
                            perror("accept");
                            continue;
                        }
                        close(acfd);
                    }
                } else {
                    char buffer[BUFSIZ];
                    ssize_t len;
                    len = read(fds[i].fd, buffer, sizeof(buffer));
                    if(len > 0) {
                        for (int j = 0; j < len; j++) {
                            printf("%c",toupper(buffer[j]));
                        }

                    } else if (len == 0) {
                        close(fds[i].fd);
                        fds[i] = fds[nfds - 1];
                        nfds--;
                        i--;
                        break;
                    } else if(len == -1) {
                        close(fds[i].fd);
                        fds[i] = fds[nfds - 1];
                        nfds--;
                        i--;
                        perror("read");
                        break;
                    }
                }
            }
        }
    }

    for (int i = 0; i < nfds; i++) {
        close(fds[i].fd);
    }
    unlink(socket_name);

    return 0;
}
