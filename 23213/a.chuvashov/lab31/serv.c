#include <ctype.h>
#include <poll.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>

#define USERS 5

char *socket_path = "./socket";

void handle_signal(int signal) {
    unlink(socket_path);
    _exit(0);
}

void doUpper(char* buf, size_t length) {
    for (size_t i = 0; i < length; i++) buf[i] = toupper(buf[i]);
}

int create_socket() {
    int descriptor;
    if ((descriptor = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("Socket failure");
        exit(-1);
    }

    struct sockaddr_un addr, client_addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);

    unlink(socket_path);
    if (bind(descriptor, (struct sockaddr* )&addr, sizeof(addr)) == -1) {
        perror("Bind failure");
        exit(-1);
    }

    if (listen(descriptor, USERS) == -1) {
        unlink(socket_path);
        perror("listen failure");
        exit(-1);
    }
    return descriptor;
}

int main() {
    signal(SIGINT, handle_signal);

    int descriptor = create_socket();
    char buffer[BUFSIZ];

    nfds_t pollIndex = 0;
    struct pollfd pollDescriptors[USERS+1];
    memset(pollDescriptors, 0, sizeof(pollDescriptors));

    pollDescriptors[0].fd = descriptor;
    pollDescriptors[0].events = POLLIN;

    while (1) {
        int result = poll(pollDescriptors, pollIndex + 1, -1);
        if (result < 0) {
            unlink("./socket");
            perror("Poll failure");
            exit(-1);
        }

        if (pollDescriptors[0].revents & POLLIN) {
            int client_descriptor;
            if ((client_descriptor = accept(descriptor, NULL, NULL)) < 0) {
                unlink(socket_path);
                perror("Accept failure");
                exit(-1);
            }

            pollDescriptors[++pollIndex].fd = client_descriptor;
            pollDescriptors[pollIndex].events = POLLIN;
            if (pollIndex == USERS) {
                pollDescriptors[0].events = 0;
            }
        }

        for (int i = 1 ; i <= pollIndex; i++) {
            if (pollDescriptors[i].fd != 0 && pollDescriptors[i].revents & POLLIN) {
                size_t byteses;
                if ((byteses = read(pollDescriptors[i].fd, buffer, BUFSIZ)) > 0) {
                    doUpper(buffer, byteses);
                    write(STDOUT_FILENO, buffer, byteses);
                } else {
                    close(pollDescriptors[i].fd);
                    for (int j = i; j < pollIndex; j++) {
                        pollDescriptors[j] = pollDescriptors[j+1];
                    }
                    pollDescriptors[pollIndex].fd = 0;
                    pollDescriptors[pollIndex--].events = 0;
                    pollDescriptors[0].events = POLLIN;
                    i--;
                }
            }
        }
    }
}
