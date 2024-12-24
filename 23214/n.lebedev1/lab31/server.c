#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <poll.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCK_PATH "socket.sock"
#define MAX_CLIENT_COUNT 20

typedef void (*handler_t)(int);

int listen_fd = -1;
int accepted_fd = -1;
int bind_flag = -1;

void fds_clean(struct pollfd* fds) {
    for (int i = 0; i < MAX_CLIENT_COUNT; i++) {
        if (fds[i].fd != 0) {
            close(fds[i].fd);
            fds[i].fd = 0;
        }
    }
}

void int_sig_handler() {
    if (listen_fd != -1) {
        close(listen_fd);
    }
    if (bind_flag == 0) {
        unlink(SOCK_PATH);
    }
    exit(0);
}

int main() {
    struct pollfd fds[MAX_CLIENT_COUNT + 1];
    char msg[BUFSIZ];
    size_t length = sizeof(msg);
    ssize_t size;
    if (signal(SIGHUP, int_sig_handler) == SIG_ERR) {
        perror("Error while setting SIGHUP signal handler");
        exit(-1);
    }
    if (signal(SIGINT, int_sig_handler) == SIG_ERR) {
        perror("Error while setting SIGINT signal handler");
        exit(-1);
    }
    if ((listen_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("Error with socket function");
        exit(-1);
    }
    printf("Created socket.\n");
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCK_PATH, sizeof(addr.sun_path) - 1);
    if ((bind_flag = bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr))) == -1) {
        perror("Error with bind");
        close(listen_fd);
        exit(-1);
    }
    printf("Created socket end point with bind.\n");
    if (listen(listen_fd, MAX_CLIENT_COUNT) == -1) {
        perror("Error with listen");
        close(listen_fd);
        unlink(SOCK_PATH);
        exit(-1);
    }
    printf("Server socket is ready for listen.\n");
    memset(fds, 0, sizeof(fds));
    fds[0].fd = listen_fd;
    fds[0].events = POLLIN;
    nfds_t clients_num = 1;
    while (1) {
        int ret = poll(fds, clients_num, -1);
        if (ret < 0) {
            perror("Error with poll");
            fds_clean(fds);
            unlink(SOCK_PATH);
            exit(EXIT_FAILURE);
        }
        if (ret > 0) {
            for (long unsigned int i = 0; i < clients_num; i++) {
                if (fds[i].revents & POLLIN) {
                    if (i == 0) {
                        if ((accepted_fd = accept(listen_fd, NULL, NULL)) == -1) {
                            perror("Error with accept");
                            fds_clean(fds);
                            unlink(SOCK_PATH);
                            exit(-1);
                        }
                        clients_num++;
                        printf("New client with descriptor = %d, length = %ld\n", accepted_fd, clients_num);
                        if (clients_num == MAX_CLIENT_COUNT + 1) {
                            fds[0].events = 0;
                        }
                        fds[clients_num - 1].fd = accepted_fd;
                        fds[clients_num - 1].events = POLLIN;
                    } else if (fds[i].fd == 0) {
                        continue;
                    } else {
                        if ((size = read(fds[i].fd, msg, length - 1)) > 0) {
                            msg[size] = '\0';
                            for (ssize_t j = 0; j < size; j++) {
                                msg[j] = toupper(msg[j]);
                            }
                            printf("%s", msg);
                        }
                        else {
                            if (size == 0) {
                                close(fds[i].fd);
                                fds[i] = fds[clients_num - 1];
                                fds[clients_num - 1].fd = 0;
                                fds[clients_num - 1].events = 0;
                                fds[clients_num - 1].revents = 0;
                                fds[0].events = POLLIN;
                                clients_num--;
                                i--;
                            }
                            if (size == -1) {
                                perror("Error with read");
                                fds_clean(fds);
                                unlink(SOCK_PATH);
                                exit(-1);
                            }
                        }
                    }
                }
            }
        }
    }
    fds_clean(fds);
    unlink(SOCK_PATH);
    exit(0);
}
